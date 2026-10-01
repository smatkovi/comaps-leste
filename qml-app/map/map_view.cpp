#include "map_view.hpp"
#include "map_engine.hpp"

#include "map/framework.hpp"

#include "drape_frontend/visual_params.hpp"

#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QTouchEvent>

#include <algorithm>
#include <cmath>

namespace
{
double g_pinchSpeed = 2.0;
double g_scrollSpeed = 2.0;

df::TouchEvent::ETouchType ToDfTouchType(QEvent::Type type)
{
  switch (type)
  {
  case QEvent::TouchBegin: return df::TouchEvent::TOUCH_DOWN;
  case QEvent::TouchEnd: return df::TouchEvent::TOUCH_UP;
  case QEvent::TouchUpdate: return df::TouchEvent::TOUCH_MOVE;
  case QEvent::TouchCancel: return df::TouchEvent::TOUCH_CANCEL;
  default: return df::TouchEvent::TOUCH_NONE;
  }
}
}  // namespace

void MapView::SetPinchSpeed(double speed)
{
  g_pinchSpeed = speed;
}

void MapView::SetScrollSpeed(double speed)
{
  g_scrollSpeed = speed;
}

MapView::MapView(QQuickItem * parent) : QQuickItem(parent)
{
  setFlag(ItemHasContents, true);
  UpdateInputFlags();

  m_frameTimer.setInterval(1000 / 60);
  connect(&m_frameTimer, &QTimer::timeout, this, &QQuickItem::update);
  connect(&MapEngine::Instance(), &MapEngine::activeViewChanged, this, [this] {
    if (active())
      m_frameTimer.start();
    else
      m_frameTimer.stop();
    update();
    emit activeChanged();
  });
}

MapView::~MapView()
{
  if (m_window)
    m_window->removeEventFilter(this);
  MapEngine::Instance().Detach(this);
}

void MapView::setPrimary(bool primary)
{
  if (m_primary == primary)
    return;
  m_primary = primary;
  emit primaryChanged();
  ScheduleReevaluate();
}

void MapView::setInteractive(bool interactive)
{
  if (m_interactive == interactive)
    return;
  m_interactive = interactive;
  UpdateInputFlags();
  emit interactiveChanged();
}

void MapView::setVisibleArea(QRectF const & area)
{
  if (m_visibleArea == area)
    return;
  m_visibleArea = area;
  emit visibleAreaChanged();
  MapEngine::Instance().ApplyVisibleArea(this);
}

bool MapView::active() const
{
  return MapEngine::Instance().IsActive(this);
}

bool MapView::IsShowing() const
{
  return isVisible() && m_window && m_window->isVisible() && m_window->isExposed() && !PixelSize().isEmpty();
}

double MapView::Ratio() const
{
  return m_window ? m_window->devicePixelRatio() : 1.0;
}

QSize MapView::PixelSize() const
{
  double const ratio = Ratio();
  return QSize(static_cast<int>(std::lround(width() * ratio)), static_cast<int>(std::lround(height() * ratio)));
}

void MapView::UpdateInputFlags()
{
  setAcceptTouchEvents(m_interactive);
  setAcceptedMouseButtons(m_interactive ? Qt::LeftButton : Qt::NoButton);
}

void MapView::ScheduleReevaluate()
{
  if (m_reevaluatePending)
    return;
  m_reevaluatePending = true;
  QMetaObject::invokeMethod(this, [this] {
    m_reevaluatePending = false;
    MapEngine::Instance().Attach(this);
  }, Qt::QueuedConnection);
}

void MapView::WatchWindow(QQuickWindow * window)
{
  if (m_window == window)
    return;
  if (m_window)
    m_window->removeEventFilter(this);
  m_window = window;
  if (m_window)
    m_window->installEventFilter(this);
  ScheduleReevaluate();
}

bool MapView::eventFilter(QObject * watched, QEvent * event)
{
  switch (event->type())
  {
  case QEvent::Expose:
  case QEvent::Show:
  case QEvent::Hide:
  case QEvent::DevicePixelRatioChange: ScheduleReevaluate(); break;
  default: break;
  }
  return QQuickItem::eventFilter(watched, event);
}

void MapView::itemChange(ItemChange change, ItemChangeData const & value)
{
  if (change == ItemSceneChange)
    WatchWindow(value.window);
  else if (change == ItemVisibleHasChanged)
    ScheduleReevaluate();
  QQuickItem::itemChange(change, value);
}

void MapView::geometryChange(QRectF const & newGeometry, QRectF const & oldGeometry)
{
  QQuickItem::geometryChange(newGeometry, oldGeometry);
  if (newGeometry.size() != oldGeometry.size())
    ScheduleReevaluate();
}

QSGNode * MapView::updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData *)
{
  auto & engine = MapEngine::Instance();
  auto * factory = engine.GetContextFactory();
  GLuint const textureId = factory && engine.IsActive(this) && factory->AcquireFrame() ? factory->GetTextureHandle() : 0;
  QSize const textureSize = textureId != 0 ? factory->GetTextureSize() : QSize();

  if (textureId == 0 || textureSize.isEmpty())
  {
    delete oldNode;
    m_textureId = 0;
    return nullptr;
  }

  auto * node = static_cast<QSGSimpleTextureNode *>(oldNode);
  if (!node)
  {
    node = new QSGSimpleTextureNode;
    node->setOwnsTexture(true);
    node->setFiltering(QSGTexture::Linear);
    node->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    m_textureId = 0;
  }

  if (textureId != m_textureId || textureSize != m_textureSize)
  {
    node->setTexture(QNativeInterface::QSGOpenGLTexture::fromNative(textureId, window(), textureSize));
    m_textureId = textureId;
    m_textureSize = textureSize;
  }

  QRectF const & texRect = factory->GetTexRect();
  node->setSourceRect(
      QRectF(0, 0, texRect.width() * textureSize.width(), texRect.height() * textureSize.height()));
  node->setRect(boundingRect());
  return node;
}

void MapView::touchEvent(QTouchEvent * event)
{
  auto const type = ToDfTouchType(event->type());
  auto const & points = event->points();
  if (!m_interactive || !active() || type == df::TouchEvent::TOUCH_NONE || points.isEmpty())
  {
    event->ignore();
    return;
  }

  double const ratio = Ratio();
  m2::PointD locations[2];
  qsizetype const count = std::min<qsizetype>(points.size(), 2);
  for (qsizetype i = 0; i < count; ++i)
    locations[i] = m2::PointD(points[i].position().x() * ratio, points[i].position().y() * ratio);

  bool const fingerDown = std::any_of(points.begin(), points.end(),
                                      [](QEventPoint const & p) { return p.state() == QEventPoint::Pressed; });
  if (fingerDown)
    m_pinchStartDistance = 0.0;

  if (count == 2)
  {
    m_scrollId = -1;
    double const distance = locations[0].Length(locations[1]);
    if (points[0].id() != m_pinchIds[0] || points[1].id() != m_pinchIds[1] || m_pinchStartDistance <= 0.0)
    {
      m_pinchIds[0] = points[0].id();
      m_pinchIds[1] = points[1].id();
      m_pinchStartDistance = distance;
      m_pinchCenter = (locations[0] + locations[1]) * 0.5;
    }
    else if (distance > 0.0)
    {
      // Zoom around the point where the pinch started: drifting fingers would pan the map, and in
      // perspective a small drift near the horizon moves the camera far away.
      double const amplified = m_pinchStartDistance * std::pow(distance / m_pinchStartDistance, g_pinchSpeed);
      m2::PointD const direction = (locations[1] - locations[0]) * (0.5 * amplified / distance);
      locations[0] = m_pinchCenter - direction;
      locations[1] = m_pinchCenter + direction;
    }
  }
  else
  {
    m_pinchStartDistance = 0.0;
    // One finger: amplify the movement beyond drape's drag threshold, so taps and the start of a drag
    // are detected exactly as without amplification.
    if (points[0].id() != m_scrollId)
    {
      m_scrollId = points[0].id();
      m_scrollOrigin = locations[0];
    }
    m2::PointD const offset = locations[0] - m_scrollOrigin;
    double const distance = offset.Length();
    double const threshold = df::VisualParams::Instance().GetDragThreshold();
    if (distance > threshold)
      locations[0] = m_scrollOrigin + offset * ((threshold + (distance - threshold) * g_scrollSpeed) / distance);
  }

  SendTouches(type, locations, static_cast<int>(count));
  if (type == df::TouchEvent::TOUCH_UP || type == df::TouchEvent::TOUCH_CANCEL)
  {
    m_pinchStartDistance = 0.0;
    m_scrollId = -1;
    m_touchOpen = false;
  }
  else
  {
    m_touchOpen = true;
  }
  event->accept();
}

void MapView::SendTouches(df::TouchEvent::ETouchType type, m2::PointD const * locations, int count)
{
  df::TouchEvent touchEvent;
  touchEvent.SetTouchType(type);
  for (int i = 0; i < count; ++i)
  {
    df::Touch touch;
    touch.m_id = i;
    touch.m_location = locations[i];
    if (i == 0)
      touchEvent.SetFirstTouch(touch);
    else
      touchEvent.SetSecondTouch(touch);
    m_lastLocations[i] = locations[i];
  }
  m_lastCount = count;
  MapEngine::Instance().GetFramework().TouchEvent(touchEvent);
}

void MapView::touchUngrabEvent()
{
  m_pinchStartDistance = 0.0;
  m_scrollId = -1;
  // Qt also calls this after every normal release. Drape only needs a cancel when a touch sequence was
  // cut off, and it must carry the touches: an empty cancel wipes drape's touches and a pending tap never fires.
  if (!m_touchOpen || m_lastCount == 0 || !active())
  {
    m_touchOpen = false;
    return;
  }
  m_touchOpen = false;
  SendTouches(df::TouchEvent::TOUCH_CANCEL, m_lastLocations, m_lastCount);
}

void MapView::SendMouse(QMouseEvent * event, int type)
{
  if (!m_interactive || !active())
  {
    event->ignore();
    return;
  }
  double const ratio = Ratio();
  df::Touch touch;
  touch.m_id = 0;
  touch.m_location = m2::PointD(event->position().x() * ratio, event->position().y() * ratio);
  df::TouchEvent touchEvent;
  touchEvent.SetTouchType(static_cast<df::TouchEvent::ETouchType>(type));
  touchEvent.SetFirstTouch(touch);
  MapEngine::Instance().GetFramework().TouchEvent(touchEvent);
  event->accept();
}

void MapView::mousePressEvent(QMouseEvent * event)
{
  SendMouse(event, df::TouchEvent::TOUCH_DOWN);
}

void MapView::mouseMoveEvent(QMouseEvent * event)
{
  SendMouse(event, df::TouchEvent::TOUCH_MOVE);
}

void MapView::mouseReleaseEvent(QMouseEvent * event)
{
  SendMouse(event, df::TouchEvent::TOUCH_UP);
}

void MapView::wheelEvent(QWheelEvent * event)
{
  if (!m_interactive || !active())
  {
    event->ignore();
    return;
  }
  double const ratio = Ratio();
  double const factor = event->angleDelta().y() / 3.0 / 360.0;
  MapEngine::Instance().GetFramework().Scale(
      std::exp(factor), m2::PointD(event->position().x() * ratio, event->position().y() * ratio), false);
  event->accept();
}
