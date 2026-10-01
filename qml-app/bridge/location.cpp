#include "location.hpp"
#include "map_engine.hpp"

#include "map/framework.hpp"

#include "geometry/mercator.hpp"

#include <QJSEngine>

namespace
{
Location * g_location = nullptr;
double constexpr kPlaceRefreshMeters = 40.0;
double constexpr kPlaceRefreshSeconds = 30.0;
}  // namespace

Location::Location() : m_service(CreateDesktopLocationService(*this))
{
  g_location = this;
}

Location::~Location()
{
  if (m_service)
    m_service->Stop();
  g_location = nullptr;
}

Location * Location::create(QQmlEngine *, QJSEngine *)
{
  QJSEngine::setObjectOwnership(g_location, QJSEngine::CppOwnership);
  return g_location;
}

void Location::Start()
{
  if (m_service)
    m_service->Start();
}

void Location::OnLocationError(location::TLocationError errorCode)
{
  QMetaObject::invokeMethod(this, [this, errorCode] {
    m_failed = true;
    MapEngine::Instance().GetFramework().OnLocationError(errorCode);
    emit changed();
  }, Qt::QueuedConnection);
}

void Location::OnLocationUpdated(location::GpsInfo const & info)
{
  QMetaObject::invokeMethod(this, [this, info] {
    m_info = info;
    m_hasFix = true;
    m_failed = false;
    MapEngine::Instance().GetFramework().OnLocationUpdate(info);
    UpdatePlace();
    emit changed();
  }, Qt::QueuedConnection);
}

void Location::UpdatePlace()
{
  auto const current = mercator::FromLatLon(m_info.m_latitude, m_info.m_longitude);
  if (m_hasPlaceInfo)
  {
    auto const last = mercator::FromLatLon(m_placeInfo.m_latitude, m_placeInfo.m_longitude);
    bool const moved = mercator::DistanceOnEarth(current, last) > kPlaceRefreshMeters;
    bool const stale = m_info.m_timestamp - m_placeInfo.m_timestamp > kPlaceRefreshSeconds;
    if (!moved && !stale)
      return;
  }
  m_placeInfo = m_info;
  m_hasPlaceInfo = true;

  auto const address = AppFramework().GetAddressAtPoint(current).FormatAddress();
  QString const place = QString::fromStdString(address);
  if (place == m_place)
    return;
  m_place = place;
  emit placeChanged();
}
