#include "map_engine.hpp"
#include "map_view.hpp"

#include "map/framework.hpp"

#include "drape/pointers.hpp"

#include "qt/qt_common/helpers.hpp"

#include "base/assert.hpp"
#include "base/logging.hpp"

#include <QOpenGLContext>
#include <QTimer>

#include <algorithm>
#include <cmath>

namespace
{
MapEngine * g_instance = nullptr;
int constexpr kCloseUpZoom = 17;
int constexpr kZoomSettleMs = 300;
}  // namespace

MapEngine::MapEngine(Framework & framework) : m_framework(framework)
{
  ASSERT(g_instance == nullptr, ());
  g_instance = this;
}

MapEngine::~MapEngine()
{
  if (m_created)
  {
    m_framework.EnterBackground();
    m_framework.SetRenderingDisabled(true);
    m_contextFactory->PrepareToShutdown();
    m_framework.DestroyDrapeEngine();
    m_contextFactory.reset();
  }
  g_instance = nullptr;
}

MapEngine & MapEngine::Instance()
{
  ASSERT(g_instance != nullptr, ());
  return *g_instance;
}

void MapEngine::Attach(MapView * view)
{
  if (std::find(m_views.begin(), m_views.end(), view) == m_views.end())
    m_views.emplace_back(view);
  Reevaluate();
}

void MapEngine::Detach(MapView * view)
{
  m_views.erase(std::remove_if(m_views.begin(), m_views.end(),
                               [view](QPointer<MapView> const & v) { return v.isNull() || v == view; }),
                m_views.end());
  if (m_active == view)
    m_active = nullptr;
  Reevaluate();
}

void MapEngine::Reevaluate()
{
  MapView * candidate = nullptr;
  for (auto const & view : m_views)
  {
    if (!view || !view->IsShowing())
      continue;
    if (view->primary())
    {
      candidate = view;
      break;
    }
    if (!candidate)
      candidate = view;
  }

  if (candidate != m_active)
  {
    if (candidate)
      Activate(candidate);
    else
      m_active = nullptr;
    emit activeViewChanged();
  }
  else if (candidate && candidate->PixelSize() != m_size)
  {
    Resize(candidate->PixelSize());
  }

  bool const foreground = m_created && m_active;
  if (foreground != m_foreground)
  {
    m_foreground = foreground;
    if (foreground)
      m_framework.EnterForeground();
    else
      m_framework.EnterBackground();
  }
}

void MapEngine::Activate(MapView * view)
{
  if (m_created && m_active && m_active->primary() && !view->primary())
    m_primaryViewport = m_framework.GetCurrentViewport();

  m_active = view;

  if (!m_created)
    Create(view->PixelSize(), view->Ratio());
  else
    Resize(view->PixelSize());

  ApplyVisibleArea(view);
  // The scale ruler is only useful where the map can be zoomed.
  m_framework.SetRulerEnabled(view->primary());

  bool const following = m_framework.GetRoutingManager().IsRoutingFollowing();
  if (view->primary())
  {
    if (m_primaryViewport && !following)
      m_framework.ShowRect(*m_primaryViewport, -1, false);
    m_primaryViewport.reset();
  }
  else if (following)
  {
    QTimer::singleShot(kZoomSettleMs, this, [this] {
      if (!m_active || m_active->primary() || !m_framework.GetRoutingManager().IsRoutingFollowing())
        return;
      int const zoom = m_framework.GetDrawScale();
      if (zoom < kCloseUpZoom)
        m_framework.Scale(std::pow(2.0, kCloseUpZoom - zoom), true);
    });
  }
  else if (auto const position = m_framework.GetCurrentPosition())
  {
    m_framework.SetViewportCenter(*position, kCloseUpZoom, false);
  }
}

void MapEngine::ApplyVisibleArea(MapView const * view)
{
  if (!m_created || view != m_active)
    return;
  double const ratio = view->Ratio();
  QRectF const area = view->visibleArea();
  if (area.isEmpty())
    m_framework.SetVisibleViewport(m2::RectD(0, 0, m_size.width(), m_size.height()));
  else
    m_framework.SetVisibleViewport(
        m2::RectD(area.left() * ratio, area.top() * ratio, area.right() * ratio, area.bottom() * ratio));
}

void MapEngine::Create(QSize pixelSize, double ratio)
{
  QOpenGLContext * shareContext = QOpenGLContext::globalShareContext();
  CHECK(shareContext != nullptr, ("Qt::AA_ShareOpenGLContexts must be set before the application exists"));
  LOG(LINFO, ("Creating the drape engine", pixelSize.width(), "x", pixelSize.height(), "ratio", ratio));

  m_framework.LoadBookmarks();
  m_contextFactory = std::make_unique<qt::common::QtOGLContextFactory>(shareContext);

  Framework::DrapeCreationParams p;
  p.m_apiVersion = qt::common::GetOpenGLApiVersion(*shareContext);
  LOG(LINFO, ("Graphics API:", p.m_apiVersion));
  p.m_surfaceWidth = pixelSize.width();
  p.m_surfaceHeight = pixelSize.height();
  p.m_visualScale = static_cast<float>(ratio);

  m_skin = std::make_unique<gui::Skin>(gui::ResolveGuiSkinFile("walking"), ratio);
  m_skin->Resize(p.m_surfaceWidth, p.m_surfaceHeight);
  m_skin->ForEach([&p](gui::EWidget widget, gui::Position const & pos) { p.m_widgetsInitInfo[widget] = pos; });

  m_framework.CreateDrapeEngine(make_ref(m_contextFactory), std::move(p));
  m_size = pixelSize;
  m_created = true;
}

void MapEngine::Resize(QSize pixelSize)
{
  if (pixelSize == m_size || pixelSize.isEmpty())
    return;

  m_size = pixelSize;
  m_framework.OnSize(pixelSize.width(), pixelSize.height());

  m_skin->Resize(pixelSize.width(), pixelSize.height());
  gui::TWidgetsLayoutInfo layout;
  m_skin->ForEach([&layout](gui::EWidget w, gui::Position const & pos) { layout[w] = pos.m_pixelPivot; });
  m_framework.SetWidgetLayout(std::move(layout));
  ApplyVisibleArea(m_active);
}
