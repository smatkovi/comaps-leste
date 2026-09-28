#include "qt/qt_common/helpers.hpp"

#include "geometry/mercator.hpp"

#include "base/logging.hpp"

#include <QtCore/QDateTime>
#include <QtGui/QSurfaceFormat>

namespace qt::common
{

bool IsLeftButton(Qt::MouseButtons buttons)
{
  return buttons & Qt::LeftButton;
}

bool IsLeftButton(QMouseEvent const * const e)
{
  return IsLeftButton(e->button()) || IsLeftButton(e->buttons());
}

bool IsRightButton(Qt::MouseButtons buttons)
{
  return buttons & Qt::RightButton;
}

bool IsRightButton(QMouseEvent const * const e)
{
  return IsRightButton(e->button()) || IsRightButton(e->buttons());
}

bool IsCommandModifier(QMouseEvent const * const e)
{
  return e->modifiers() & Qt::ControlModifier;
}

bool IsShiftModifier(QMouseEvent const * const e)
{
  return e->modifiers() & Qt::ShiftModifier;
}

bool IsAltModifier(QMouseEvent const * const e)
{
  return e->modifiers() & Qt::AltModifier;
}

location::GpsInfo MakeGpsInfo(m2::PointD const & point)
{
  location::GpsInfo info;
  info.m_source = location::EUser;
  info.m_latitude = mercator::YToLat(point.y);
  info.m_longitude = mercator::XToLon(point.x);
  info.m_horizontalAccuracy = 10;
  info.m_timestamp = QDateTime::currentMSecsSinceEpoch() / 1000.0;
  return info;
}

void SetDefaultSurfaceFormat(QString const & /* platformName */)
{
  // OpenGL ES 3.0: no profile, RGBA8, 24-bit depth, 8-bit stencil.
  QSurfaceFormat fmt;
  fmt.setProfile(QSurfaceFormat::NoProfile);
  fmt.setRenderableType(QSurfaceFormat::OpenGLES);
  fmt.setVersion(3, 0);
  fmt.setRedBufferSize(8);
  fmt.setGreenBufferSize(8);
  fmt.setBlueBufferSize(8);
  fmt.setAlphaBufferSize(8);
  fmt.setDepthBufferSize(24);
  fmt.setStencilBufferSize(8);
  fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  fmt.setSwapInterval(1);
#ifdef ENABLE_OPENGL_DIAGNOSTICS
  fmt.setOption(QSurfaceFormat::DebugContext);
#endif
  LOG(LINFO, ("Set default surface format to OpenGL ES 3.0"));
  QSurfaceFormat::setDefaultFormat(fmt);
}

}  // namespace qt::common
