#include "map_view.hpp"
#include "map_engine.hpp"

#include "map/framework.hpp"

#include "drape_frontend/visual_params.hpp"

#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QTouchEvent>

#include <algorithm>
#include <cmath>

namespace
{
double g_pinchSpeed = 2.0;
double g_scrollSpeed = 2.0;
// COMAPS_TOUCH_DEBUG: every touch event and what drape gets, to stderr
bool const g_touchDebug = qEnvironmentVariableIsSet("COMAPS_TOUCH_DEBUG");

char const * TouchTypeName(df::TouchEvent::ETouchType type)
{
  switch (type)
  {
  case df::TouchEvent::TOUCH_DOWN: return "down";
  case df::TouchEvent::TOUCH_MOVE: return "move";
  case df::TouchEvent::TOUCH_UP: return "up";
  case df::TouchEvent::TOUCH_CANCEL: return "cancel";
  default: return "none";
  }
}

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
  // Repaint only when drape has rendered a new frame: an idle map then costs no GPU time
  // (scene graph render + swap), which matters on slow devices.
  connect(&m_frameTimer, &QTimer::timeout, this, [this] {
    auto * factory = MapEngine::Instance().GetContextFactory();
    if (factory == nullptr || factory->HasNewFrame())
      update();
  });
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
  if (m_renderWindow)
    m_renderWindow->removeEventFilter(this);
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
  if (!isVisible() || !m_window || PixelSize().isEmpty())
    return false;
  // Inside a QQuickWidget the scene's window is offscreen, neither visible nor exposed; the widget's
  // window is what shows.
  if (auto * renderWindow = QQuickRenderControl::renderWindowFor(m_window))
    return renderWindow->isVisible() && renderWindow->isExposed();
  return m_window->isVisible() && m_window->isExposed();
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
    WatchRenderWindow();
    if (m_renderWindow || IsShowing())
      disconnect(m_frameConnection);
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
  disconnect(m_frameConnection);
  if (m_window)
  {
    m_window->installEventFilter(this);
    // A QQuickWidget marks its offscreen window visible without a Show event, and the window it is
    // shown in only exists once the widget is: check again after frames until that is known.
    m_frameConnection = connect(m_window, &QQuickWindow::afterFrameEnd, this, &MapView::ScheduleReevaluate);
  }
  ScheduleReevaluate();
}

void MapView::WatchRenderWindow()
{
  QWindow * renderWindow = m_window ? QQuickRenderControl::renderWindowFor(m_window) : nullptr;
  if (m_renderWindow == renderWindow)
    return;
  if (m_renderWindow)
    m_renderWindow->removeEventFilter(this);
  m_renderWindow = renderWindow;
  if (m_renderWindow)
    m_renderWindow->installEventFilter(this);
}

bool MapView::eventFilter(QObject * watched, QEvent * event)
{
  switch (event->type())
  {
  case QEvent::Expose:
  case QEvent::Show:
  case QEvent::Hide:
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
  case QEvent::DevicePixelRatioChange:
#endif
    ScheduleReevaluate(); break;
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
  auto const & points = event->points();
  if (!m_interactive || !active() || ToDfTouchType(event->type()) == df::TouchEvent::TOUCH_NONE || points.isEmpty())
  {
    event->ignore();
    return;
  }
  if (g_touchDebug)
  {
    QString line;
    for (auto const & p : points)
      line += QString(" %1:%2(%3,%4)").arg(p.id()).arg(int(p.state())).arg(p.position().x(), 0, 'f', 0).arg(p.position().y(), 0, 'f', 0);
    qInfo("touch qt=%d%s", int(event->type()), qPrintable(line));
  }

  // Drape follows two fingers by their ids: Qt keeps neither the order of the points (on X11 a new finger
  // comes first) nor their number (a third finger is ignored here). A finger that comes is a down and one
  // that goes an up, as on Android, so drape switches between tap, drag and scale on the change of fingers.
  double const ratio = Ratio();
  auto const pointOf = [&points](int id) -> QEventPoint const * {
    for (auto const & p : points)
      if (p.id() == id)
        return &p;
    return nullptr;
  };
  auto const realOf = [ratio](QEventPoint const & p) {
    return m2::PointD(p.position().x() * ratio, p.position().y() * ratio);
  };
  m2::PointD real[2], sent[2];

  if (event->type() == QEvent::TouchCancel)
  {
    if (m_fingerCount > 0)
    {
      for (int i = 0; i < m_fingerCount; ++i)
        sent[i] = LastSent(m_fingerIds[i], m2::PointD());
      SendTouches(df::TouchEvent::TOUCH_CANCEL, sent, m_fingerIds, m_fingerCount);
    }
    m_fingerCount = 0;
    m_phaseCount = 0;
    m_touchOpen = false;
    event->accept();
    return;
  }

  // Fingers lifted: an up with all fingers where drape last saw them, then the rest go on.
  bool lifted = false;
  for (int i = 0; i < m_fingerCount; ++i)
  {
    QEventPoint const * p = pointOf(m_fingerIds[i]);
    lifted = lifted || event->type() == QEvent::TouchEnd || (p && p->state() == QEventPoint::Released);
  }
  if (lifted)
  {
    for (int i = 0; i < m_fingerCount; ++i)
      sent[i] = LastSent(m_fingerIds[i], pointOf(m_fingerIds[i]) ? realOf(*pointOf(m_fingerIds[i])) : m2::PointD());
    SendTouches(df::TouchEvent::TOUCH_UP, sent, m_fingerIds, m_fingerCount);
    int kept = 0;
    for (int i = 0; i < m_fingerCount; ++i)
    {
      QEventPoint const * p = pointOf(m_fingerIds[i]);
      bool const gone = event->type() == QEvent::TouchEnd || (p && p->state() == QEventPoint::Released);
      if (!gone)
        m_fingerIds[kept++] = m_fingerIds[i];
    }
    m_fingerCount = kept;
    m_phaseCount = 0;
  }

  // Fingers put down: a down with the new ones where they are and the others where drape last saw them.
  bool added = false;
  for (auto const & p : points)
  {
    if (p.state() != QEventPoint::Pressed || m_fingerCount == 2)
      continue;
    if (std::find(m_fingerIds, m_fingerIds + m_fingerCount, p.id()) != m_fingerIds + m_fingerCount)
      continue;
    m_fingerIds[m_fingerCount++] = p.id();
    added = true;
  }
  if (added)
  {
    for (int i = 0; i < m_fingerCount; ++i)
    {
      QEventPoint const * p = pointOf(m_fingerIds[i]);
      real[i] = p ? realOf(*p) : LastSent(m_fingerIds[i], m2::PointD());
      sent[i] = LastSent(m_fingerIds[i], real[i]);
    }
    SendTouches(df::TouchEvent::TOUCH_DOWN, sent, m_fingerIds, m_fingerCount);
    StartTouchPhase(real, sent);
  }

  if (!lifted && !added && m_fingerCount > 0)
  {
    for (int i = 0; i < m_fingerCount; ++i)
    {
      QEventPoint const * p = pointOf(m_fingerIds[i]);
      real[i] = p ? realOf(*p) : LastSent(m_fingerIds[i], m2::PointD());
    }
    bool const samePhase = m_phaseCount == m_fingerCount && m_phaseIds[0] == m_fingerIds[0] &&
                           (m_fingerCount < 2 || m_phaseIds[1] == m_fingerIds[1]);
    if (!samePhase)
    {
      // The first movement after a finger went: go on from where drape last saw the others.
      for (int i = 0; i < m_fingerCount; ++i)
        sent[i] = LastSent(m_fingerIds[i], real[i]);
      StartTouchPhase(real, sent);
    }
    else if (m_fingerCount == 2)
    {
      // Zoom around the point where the pinch started: drifting fingers would pan the map, and in
      // perspective a small drift near the horizon moves the camera far away. The fingers' distance
      // changes by its own ratio to the power of the pinch speed, the direction turns with the fingers.
      m2::PointD const realStart = m_phaseReal[1] - m_phaseReal[0];
      m2::PointD const realNow = real[1] - real[0];
      m2::PointD const sentStart = m_phaseSent[1] - m_phaseSent[0];
      double const startLength = realStart.Length();
      double const nowLength = realNow.Length();
      double const sentLength = sentStart.Length();
      if (startLength > 0.0 && nowLength > 0.0 && sentLength > 0.0)
      {
        double const amplified = sentLength * std::pow(nowLength / startLength, g_pinchSpeed);
        double const turn = std::atan2(realNow.y, realNow.x) - std::atan2(realStart.y, realStart.x);
        double const c = std::cos(turn), s = std::sin(turn);
        m2::PointD const unit = sentStart * (1.0 / sentLength);
        m2::PointD const half = m2::PointD(unit.x * c - unit.y * s, unit.x * s + unit.y * c) * (0.5 * amplified);
        m2::PointD const center = (m_phaseSent[0] + m_phaseSent[1]) * 0.5;
        sent[0] = center - half;
        sent[1] = center + half;
      }
      else
      {
        sent[0] = LastSent(m_fingerIds[0], real[0]);
        sent[1] = LastSent(m_fingerIds[1], real[1]);
      }
    }
    else
    {
      // One finger: amplify the movement beyond drape's drag threshold, so taps and the start of a drag
      // are detected exactly as without amplification.
      m2::PointD const offset = real[0] - m_phaseReal[0];
      double const distance = offset.Length();
      double const threshold = df::VisualParams::Instance().GetDragThreshold();
      sent[0] = m_phaseSent[0] +
                (distance > threshold ? offset * ((threshold + (distance - threshold) * g_scrollSpeed) / distance)
                                      : offset);
    }
    SendTouches(df::TouchEvent::TOUCH_MOVE, sent, m_fingerIds, m_fingerCount);
  }

  m_touchOpen = m_fingerCount > 0;
  if (!m_touchOpen)
    m_phaseCount = 0;
  event->accept();
}

void MapView::StartTouchPhase(m2::PointD const * real, m2::PointD const * sent)
{
  m_phaseCount = m_fingerCount;
  for (int i = 0; i < m_fingerCount; ++i)
  {
    m_phaseIds[i] = m_fingerIds[i];
    m_phaseReal[i] = real[i];
    m_phaseSent[i] = sent[i];
  }
}

m2::PointD MapView::LastSent(int id, m2::PointD const & otherwise) const
{
  for (int i = 0; i < m_lastCount; ++i)
    if (m_lastIds[i] == id)
      return m_lastLocations[i];
  return otherwise;
}

void MapView::SendTouches(df::TouchEvent::ETouchType type, m2::PointD const * locations, int const * ids, int count)
{
  df::TouchEvent touchEvent;
  touchEvent.SetTouchType(type);
  for (int i = 0; i < count; ++i)
  {
    df::Touch touch;
    touch.m_id = ids[i];
    touch.m_location = locations[i];
    if (i == 0)
      touchEvent.SetFirstTouch(touch);
    else
      touchEvent.SetSecondTouch(touch);
    m_lastLocations[i] = locations[i];
    m_lastIds[i] = ids[i];
  }
  m_lastCount = count;
  if (g_touchDebug)
    qInfo("  drape %s %d: %d(%.0f,%.0f) %d(%.0f,%.0f)", TouchTypeName(type), count, ids[0], locations[0].x,
          locations[0].y, count > 1 ? ids[1] : -1, count > 1 ? locations[1].x : 0.0, count > 1 ? locations[1].y : 0.0);
  MapEngine::Instance().GetFramework().TouchEvent(touchEvent);
}

void MapView::touchUngrabEvent()
{
  // Qt also calls this after every normal release. Drape only needs a cancel when a touch sequence was
  // cut off, and it must carry the touches: an empty cancel wipes drape's touches and a pending tap never fires.
  bool const open = m_touchOpen && m_lastCount > 0 && active();
  m_touchOpen = false;
  m_fingerCount = 0;
  m_phaseCount = 0;
  if (open)
  {
    int ids[2] = {m_lastIds[0], m_lastIds[1]};
    m2::PointD locations[2] = {m_lastLocations[0], m_lastLocations[1]};
    SendTouches(df::TouchEvent::TOUCH_CANCEL, locations, ids, m_lastCount);
  }
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
