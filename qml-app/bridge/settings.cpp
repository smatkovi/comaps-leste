#include "settings.hpp"
#include "map_engine.hpp"
#include "map_view.hpp"

#include "map/framework.hpp"

#include "platform/measurement_utils.hpp"
#include "platform/platform.hpp"
#include "platform/settings.hpp"

#include <QJSEngine>

namespace
{
Settings * g_settings = nullptr;
double constexpr kDefaultPinchSpeed = 2.0;
std::string_view constexpr kPinchSpeedKey = "PinchZoomSpeed";
double constexpr kDefaultScrollSpeed = 2.0;
std::string_view constexpr kScrollSpeedKey = "ScrollSpeed";
}  // namespace

Settings::Settings(Framework & framework) : m_framework(framework)
{
  g_settings = this;
  MapView::SetPinchSpeed(pinchSpeed());
  MapView::SetScrollSpeed(scrollSpeed());
}

Settings::~Settings()
{
  g_settings = nullptr;
}

Settings * Settings::create(QQmlEngine *, QJSEngine *)
{
  QJSEngine::setObjectOwnership(g_settings, QJSEngine::CppOwnership);
  return g_settings;
}

int Settings::units() const
{
  return measurement_utils::GetMeasurementUnits() == measurement_utils::Units::Imperial ? 1 : 0;
}

void Settings::setUnits(int units)
{
  settings::Set(settings::kMeasurementUnits,
                units == 1 ? measurement_utils::Units::Imperial : measurement_utils::Units::Metric);
  m_framework.SetupMeasurementSystem();
  emit changed();
}

double Settings::labelScale()
{
  return m_framework.LoadFontScaleFactor();
}

void Settings::setLabelScale(double scale)
{
  if (!MapEngine::Instance().IsCreated())
    return;
  m_framework.SetFontScaleFactor(scale);
  emit changed();
}

double Settings::iconScale()
{
  return m_framework.LoadSymbolScale();
}

void Settings::setIconScale(double scale)
{
  if (!MapEngine::Instance().IsCreated())
    return;
  m_framework.SetSymbolScale(scale);
  emit changed();
}

bool Settings::transliteration()
{
  return m_framework.LoadTransliteration();
}

void Settings::setTransliteration(bool on)
{
  m_framework.SaveTransliteration(on);
  m_framework.AllowTransliteration(on);
  emit changed();
}

bool Settings::bookmarkLabels() const
{
  return Framework::GetShowBookmarkLabels();
}

void Settings::setBookmarkLabels(bool on)
{
  m_framework.SetShowBookmarkLabels(on);
  emit changed();
}

QString Settings::mapLanguage()
{
  auto const code = m_framework.GetCustomMapLanguageCode();
  if (!code)
    return QStringLiteral("auto");
  if (*code == "default")
    return QStringLiteral("local");
  return QString::fromStdString(*code);
}

void Settings::setMapLanguage(QString const & language)
{
  if (language == QLatin1String("auto"))
    m_framework.SetCustomMapLanguageCode();
  else if (language == QLatin1String("local"))
    m_framework.SetCustomMapLanguageCode("default");
  else
    m_framework.SetCustomMapLanguageCode(language.toStdString());
  emit changed();
}

bool Settings::perspective()
{
  return m_framework.LoadPerspectiveView();
}

void Settings::setPerspective(bool on)
{
  m_framework.SetPerspectiveView(on);
  emit changed();
}

bool Settings::navigationPerspective()
{
  bool allow3d = false;
  bool buildings = true;
  m_framework.Load3dMode(allow3d, buildings);
  return allow3d;
}

void Settings::setNavigationPerspective(bool on)
{
  bool allow3d = false;
  bool buildings = true;
  m_framework.Load3dMode(allow3d, buildings);
  m_framework.Save3dMode(on, buildings);
  m_framework.Allow3dMode(on, buildings);
  emit changed();
}

bool Settings::autoZoom()
{
  return m_framework.LoadAutoZoom();
}

void Settings::setAutoZoom(bool on)
{
  m_framework.SaveAutoZoom(on);
  m_framework.AllowAutoZoom(on);
  emit changed();
}

double Settings::pinchSpeed() const
{
  double speed = kDefaultPinchSpeed;
  settings::TryGet(kPinchSpeedKey, speed);
  return speed;
}

void Settings::setPinchSpeed(double speed)
{
  settings::Set(kPinchSpeedKey, speed);
  MapView::SetPinchSpeed(speed);
  emit changed();
}

double Settings::scrollSpeed() const
{
  double speed = kDefaultScrollSpeed;
  settings::TryGet(kScrollSpeedKey, speed);
  return speed;
}

void Settings::setScrollSpeed(double speed)
{
  settings::Set(kScrollSpeedKey, speed);
  MapView::SetScrollSpeed(speed);
  emit changed();
}

QString Settings::version() const
{
  return QString::fromStdString(GetPlatform().Version());
}
