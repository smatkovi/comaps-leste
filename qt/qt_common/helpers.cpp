#include "qt/qt_common/helpers.hpp"

#include "geometry/mercator.hpp"

#include "base/logging.hpp"

#include "std/target_os.hpp"

#include <QtCore/QDateTime>
#include <QtGui/QOpenGLContext>
#include <QtGui/QSurfaceFormat>

#include <cstdlib>
#include <cstring>

#if defined(OMIM_OS_LINUX)
#include <dlfcn.h>
#endif

namespace qt::common
{
namespace
{
#if defined(OMIM_OS_LINUX)
// Asks EGL whether any config can render with OpenGL ES 3. Old GPUs (e.g. PowerVR SGX5xx) support only ES 2.
bool IsEGLOpenGLES3Available()
{
  void * egl = dlopen("libEGL.so.1", RTLD_LAZY | RTLD_LOCAL);
  if (egl == nullptr)
    return true;  // Can't check, let Qt try OpenGL ES 3.

  using TGetDisplay = void * (*)(void *);
  using TInitialize = unsigned (*)(void *, int *, int *);
  using TChooseConfig = unsigned (*)(void *, int const *, void **, int, int *);
  using TTerminate = unsigned (*)(void *);
  auto const getDisplay = reinterpret_cast<TGetDisplay>(dlsym(egl, "eglGetDisplay"));
  auto const initialize = reinterpret_cast<TInitialize>(dlsym(egl, "eglInitialize"));
  auto const chooseConfig = reinterpret_cast<TChooseConfig>(dlsym(egl, "eglChooseConfig"));
  auto const terminate = reinterpret_cast<TTerminate>(dlsym(egl, "eglTerminate"));

  bool result = true;
  void * display = getDisplay && initialize && chooseConfig && terminate ? getDisplay(nullptr /* EGL_DEFAULT_DISPLAY */)
                                                                         : nullptr;
  if (display != nullptr && initialize(display, nullptr, nullptr))
  {
    int constexpr kEGLRenderableType = 0x3040;
    int constexpr kEGLOpenGLES3Bit = 0x40;
    int constexpr kEGLNone = 0x3038;
    int const attribs[] = {kEGLRenderableType, kEGLOpenGLES3Bit, kEGLNone};
    int configsCount = 0;
    result = chooseConfig(display, attribs, nullptr, 0, &configsCount) && configsCount > 0;
    terminate(display);
  }
  dlclose(egl);
  return result;
}
#endif

// OpenGL ES 3.0 is used when available, the legacy OpenGL ES 2.0 renderer otherwise.
// The COMAPS_GLES_VERSION=2 (or 3) environment variable overrides the detection.
int ChooseOpenGLESMajorVersion()
{
  if (char const * version = std::getenv("COMAPS_GLES_VERSION"))
  {
    if (std::strcmp(version, "2") == 0)
      return 2;
    if (std::strcmp(version, "3") == 0)
      return 3;
    LOG(LWARNING, ("Unsupported COMAPS_GLES_VERSION value:", version));
  }

#if defined(OMIM_OS_LINUX)
  return IsEGLOpenGLES3Available() ? 3 : 2;
#else
  return 3;
#endif
}
}  // namespace

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
  // OpenGL ES 3.0 (or 2.0): no profile, RGBA8, 24-bit depth, 8-bit stencil.
  int const majorVersion = ChooseOpenGLESMajorVersion();
  QSurfaceFormat fmt;
  fmt.setProfile(QSurfaceFormat::NoProfile);
  fmt.setRenderableType(QSurfaceFormat::OpenGLES);
  fmt.setVersion(majorVersion, 0);
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
  LOG(LINFO, ("Set default surface format to OpenGL ES, major version", majorVersion));
  QSurfaceFormat::setDefaultFormat(fmt);
}

dp::ApiVersion GetOpenGLApiVersion(QOpenGLContext const & context)
{
  if (context.format().majorVersion() < 3 || QSurfaceFormat::defaultFormat().majorVersion() < 3)
    return dp::ApiVersion::OpenGLES2;
  return dp::ApiVersion::OpenGLES3;
}
}  // namespace qt::common
