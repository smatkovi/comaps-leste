#pragma once

#include "drape/drape_global.hpp"

#include "platform/location.hpp"

#include <QtGui/QMouseEvent>

class QOpenGLContext;

namespace qt::common
{
bool IsLeftButton(Qt::MouseButtons buttons);
bool IsLeftButton(QMouseEvent const * const e);

bool IsRightButton(Qt::MouseButtons buttons);
bool IsRightButton(QMouseEvent const * const e);

bool IsCommandModifier(QMouseEvent const * const e);
bool IsShiftModifier(QMouseEvent const * const e);
bool IsAltModifier(QMouseEvent const * const e);

struct Hotkey
{
  Hotkey() = default;
  Hotkey(QKeySequence const & key, char const * slot) : m_key(key), m_slot(slot) {}

  QKeySequence m_key = 0;
  char const * m_slot = nullptr;
};

location::GpsInfo MakeGpsInfo(m2::PointD const & point);

void SetDefaultSurfaceFormat(QString const & platformName);

// The legacy OpenGL ES 2.0 renderer is used when only ES 2.0 was requested by SetDefaultSurfaceFormat
// (no ES 3 support detected, or forced with COMAPS_GLES_VERSION=2) or |context| is below ES 3.0.
// Some drivers return a newer context for an ES 2.0 request, so both are checked.
dp::ApiVersion GetOpenGLApiVersion(QOpenGLContext const & context);
}  // namespace qt::common
