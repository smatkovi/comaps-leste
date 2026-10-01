#include "routing.hpp"
#include "format.hpp"

#include "map/framework.hpp"
#include "map/place_page_info.hpp"
#include "map/routing_manager.hpp"
#include "map/routing_mark.hpp"

#include "routing/following_info.hpp"
#include "routing/router.hpp"
#include "routing/routing_options.hpp"
#include "routing/turns.hpp"

#include "storage/storage.hpp"

#include "platform/location.hpp"
#include "platform/measurement_utils.hpp"

#include "geometry/mercator.hpp"

#include <QDateTime>
#include <QJSEngine>

#include <cmath>

namespace
{
Routing * g_routing = nullptr;
double constexpr kThenDistanceMeters = 300.0;

using routing::turns::CarDirection;
using routing::turns::PedestrianDirection;

QString CarGlyph(CarDirection direction)
{
  switch (direction)
  {
  case CarDirection::TurnRight: return QStringLiteral("turn_right");
  case CarDirection::TurnSharpRight: return QStringLiteral("turn_sharp_right");
  case CarDirection::TurnSlightRight: return QStringLiteral("turn_slight_right");
  case CarDirection::TurnLeft: return QStringLiteral("turn_left");
  case CarDirection::TurnSharpLeft: return QStringLiteral("turn_sharp_left");
  case CarDirection::TurnSlightLeft: return QStringLiteral("turn_slight_left");
  case CarDirection::UTurnLeft: return QStringLiteral("u_turn_left");
  case CarDirection::UTurnRight: return QStringLiteral("u_turn_right");
  case CarDirection::EnterRoundAbout:
  case CarDirection::StayOnRoundAbout:
  case CarDirection::LeaveRoundAbout: return QStringLiteral("roundabout_right");
  case CarDirection::ReachedYourDestination: return QStringLiteral("sports_score");
  case CarDirection::ExitHighwayToLeft: return QStringLiteral("ramp_left");
  case CarDirection::ExitHighwayToRight: return QStringLiteral("ramp_right");
  case CarDirection::None:
  case CarDirection::GoStraight:
  case CarDirection::StartAtEndOfStreet:
  case CarDirection::Count: break;
  }
  return QStringLiteral("straight");
}

QString CarText(CarDirection direction, int exitNumber)
{
  switch (direction)
  {
  case CarDirection::TurnRight: return QObject::tr("Turn right");
  case CarDirection::TurnSharpRight: return QObject::tr("Turn sharp right");
  case CarDirection::TurnSlightRight: return QObject::tr("Bear right");
  case CarDirection::TurnLeft: return QObject::tr("Turn left");
  case CarDirection::TurnSharpLeft: return QObject::tr("Turn sharp left");
  case CarDirection::TurnSlightLeft: return QObject::tr("Bear left");
  case CarDirection::UTurnLeft:
  case CarDirection::UTurnRight: return QObject::tr("Make a U-turn");
  case CarDirection::EnterRoundAbout:
  case CarDirection::StayOnRoundAbout:
    return exitNumber > 0 ? QObject::tr("At the roundabout, take exit %1").arg(exitNumber)
                          : QObject::tr("Enter the roundabout");
  case CarDirection::LeaveRoundAbout: return QObject::tr("Leave the roundabout");
  case CarDirection::ReachedYourDestination: return QObject::tr("Arrive at your destination");
  case CarDirection::ExitHighwayToLeft: return QObject::tr("Take the exit on the left");
  case CarDirection::ExitHighwayToRight: return QObject::tr("Take the exit on the right");
  case CarDirection::None:
  case CarDirection::GoStraight:
  case CarDirection::StartAtEndOfStreet:
  case CarDirection::Count: break;
  }
  return QObject::tr("Continue straight");
}

QString PedestrianGlyph(PedestrianDirection direction)
{
  switch (direction)
  {
  case PedestrianDirection::TurnRight: return QStringLiteral("turn_right");
  case PedestrianDirection::TurnLeft: return QStringLiteral("turn_left");
  case PedestrianDirection::ReachedYourDestination: return QStringLiteral("sports_score");
  case PedestrianDirection::None:
  case PedestrianDirection::GoStraight:
  case PedestrianDirection::Count: break;
  }
  return QStringLiteral("straight");
}

QString PedestrianText(PedestrianDirection direction)
{
  switch (direction)
  {
  case PedestrianDirection::TurnRight: return QObject::tr("Turn right");
  case PedestrianDirection::TurnLeft: return QObject::tr("Turn left");
  case PedestrianDirection::ReachedYourDestination: return QObject::tr("Arrive at your destination");
  case PedestrianDirection::None:
  case PedestrianDirection::GoStraight:
  case PedestrianDirection::Count: break;
  }
  return QObject::tr("Continue straight");
}

QString ErrorText(routing::RouterResultCode code)
{
  using routing::RouterResultCode;
  switch (code)
  {
  case RouterResultCode::NoCurrentPosition: return QObject::tr("Waiting for your location");
  case RouterResultCode::StartPointNotFound: return QObject::tr("No road near the start point");
  case RouterResultCode::EndPointNotFound: return QObject::tr("No road near the destination");
  case RouterResultCode::IntermediatePointNotFound: return QObject::tr("No road near a stop");
  case RouterResultCode::NeedMoreMaps: return QObject::tr("More maps are needed for this route");
  case RouterResultCode::RouteFileNotExist:
  case RouterResultCode::InconsistentMWMandRoute:
  case RouterResultCode::FileTooOld: return QObject::tr("Download or update the maps along this route");
  case RouterResultCode::TransitRouteNotFoundNoNetwork: return QObject::tr("No public transport here");
  case RouterResultCode::TransitRouteNotFoundTooLongPedestrian:
    return QObject::tr("The walk to public transport is too long");
  default: return QObject::tr("No route found");
  }
}

routing::VehicleType ToVehicle(routing::RouterType type)
{
  switch (type)
  {
  case routing::RouterType::Pedestrian: return routing::VehicleType::Pedestrian;
  case routing::RouterType::Bicycle: return routing::VehicleType::Bicycle;
  case routing::RouterType::Transit: return routing::VehicleType::Transit;
  default: return routing::VehicleType::Car;
  }
}
}  // namespace

Routing::Routing(Framework & framework) : m_framework(framework)
{
  g_routing = this;

  m_framework.GetRoutingManager().SetRouteBuildingListener(
      [this](routing::RouterResultCode code, storage::CountriesSet const & absent) {
        QMetaObject::invokeMethod(this, [this, code, absent] { OnRouteBuilt(code, absent); }, Qt::QueuedConnection);
      });

  m_timer.setInterval(1000);
  connect(&m_timer, &QTimer::timeout, this, &Routing::UpdateProgress);

  // Without a fix the start point is handed to drape as the position, which arrives asynchronously.
  // Following before drape has it animates to its default position at 0,0.
  m_followTimer.setInterval(50);
  connect(&m_followTimer, &QTimer::timeout, this, [this] {
    if (!m_framework.GetCurrentPosition())
      return;
    m_followTimer.stop();
    Follow();
  });
}

Routing::~Routing()
{
  m_framework.GetRoutingManager().SetRouteBuildingListener({});
  g_routing = nullptr;
}

Routing * Routing::create(QQmlEngine *, QJSEngine *)
{
  QJSEngine::setObjectOwnership(g_routing, QJSEngine::CppOwnership);
  return g_routing;
}

int Routing::routerType() const
{
  switch (m_framework.GetRoutingManager().GetRouter())
  {
  case routing::RouterType::Pedestrian: return Walk;
  case routing::RouterType::Bicycle: return Bike;
  case routing::RouterType::Transit: return Transit;
  default: return Car;
  }
}

void Routing::setRouterType(int type)
{
  if (type == routerType())
    return;
  routing::RouterType router = routing::RouterType::Vehicle;
  switch (type)
  {
  case Walk: router = routing::RouterType::Pedestrian; break;
  case Bike: router = routing::RouterType::Bicycle; break;
  case Transit: router = routing::RouterType::Transit; break;
  default: break;
  }
  m_framework.GetRoutingManager().SetRouter(router);
  emit routerTypeChanged();
  emit optionsChanged();
  rebuild();
}

bool Routing::HasOption(int option) const
{
  auto const vehicle = ToVehicle(m_framework.GetRoutingManager().GetRouter());
  return routing::RoutingOptions::LoadOptionsFromSettings(vehicle).Has(static_cast<routing::RoutingOptions::Option>(option));
}

void Routing::SetOption(int option, bool on)
{
  auto const vehicle = ToVehicle(m_framework.GetRoutingManager().GetRouter());
  auto options = routing::RoutingOptions::LoadOptionsFromSettings(vehicle);
  auto const road = static_cast<routing::RoutingOptions::Option>(option);
  if (on)
    options.Add(road);
  else
    options.Remove(road);
  options.setVehicleType(vehicle);
  routing::RoutingOptions::SaveOptionsToSettings(options);
  emit optionsChanged();
  rebuild();
}

bool Routing::avoidToll() const { return HasOption(routing::RoutingOptions::Toll); }
void Routing::setAvoidToll(bool on) { SetOption(routing::RoutingOptions::Toll, on); }
bool Routing::avoidMotorway() const { return HasOption(routing::RoutingOptions::Motorway); }
void Routing::setAvoidMotorway(bool on) { SetOption(routing::RoutingOptions::Motorway, on); }
bool Routing::avoidFerry() const { return HasOption(routing::RoutingOptions::Ferry); }
void Routing::setAvoidFerry(bool on) { SetOption(routing::RoutingOptions::Ferry, on); }
bool Routing::avoidDirty() const { return HasOption(routing::RoutingOptions::Dirty); }
void Routing::setAvoidDirty(bool on) { SetOption(routing::RoutingOptions::Dirty, on); }
bool Routing::avoidSteps() const { return HasOption(routing::RoutingOptions::Steps); }
void Routing::setAvoidSteps(bool on) { SetOption(routing::RoutingOptions::Steps, on); }

void Routing::SetState(QString const & state)
{
  if (m_state == state)
    return;
  m_state = state;
  if (m_state == QLatin1String("idle"))
    m_timer.stop();
  else
    m_timer.start();
  emit stateChanged();
}

void Routing::AddSelectionPoint(bool isStart)
{
  if (!m_framework.HasPlacePageInfo())
    return;

  auto const & info = m_framework.GetCurrentPlacePageInfo();
  auto & manager = m_framework.GetRoutingManager();
  auto const type = isStart ? RouteMarkType::Start : RouteMarkType::Finish;

  RouteMarkData point;
  point.m_title = info.GetTitle();
  point.m_subTitle = info.GetSubtitle();
  point.m_pointType = type;
  point.m_isMyPosition = info.IsMyPosition();
  point.m_position = info.GetMercator();

  manager.RemoveRoutePoint(type);
  manager.AddRoutePoint(std::move(point));
  m_framework.DeactivateMapSelection();
}

void Routing::AddMyPosition(RouteMarkType type)
{
  auto & manager = m_framework.GetRoutingManager();
  RouteMarkData point;
  point.m_pointType = type;
  point.m_isMyPosition = true;
  if (auto const position = m_framework.GetCurrentPosition())
    point.m_position = *position;
  manager.RemoveRoutePoint(type);
  manager.AddRoutePoint(std::move(point));
}

void Routing::EnsureMyPositionStart()
{
  for (auto const & point : m_framework.GetRoutingManager().GetRoutePoints())
    if (point.m_pointType == RouteMarkType::Start)
      return;
  AddMyPosition(RouteMarkType::Start);
}

void Routing::useMyPosition(bool isStart)
{
  AddMyPosition(isStart ? RouteMarkType::Start : RouteMarkType::Finish);
  setPickTarget({});
  SetState(QStringLiteral("planning"));
  UpdatePoints();
  rebuild();
}

void Routing::setPickTarget(QString const & target)
{
  if (m_pickTarget == target)
    return;
  m_pickTarget = target;
  emit pickTargetChanged();
}

void Routing::useSelectionForPick()
{
  bool const isStart = m_pickTarget == QLatin1String("start");
  setPickTarget({});
  if (isStart)
    routeFromSelection();
  else
    routeToSelection();
}

void Routing::routeToSelection()
{
  AddSelectionPoint(false);
  EnsureMyPositionStart();
  SetState(QStringLiteral("planning"));
  UpdatePoints();
  rebuild();
}

void Routing::routeFromSelection()
{
  AddSelectionPoint(true);
  SetState(QStringLiteral("planning"));
  UpdatePoints();
  rebuild();
}

void Routing::swap()
{
  auto & manager = m_framework.GetRoutingManager();
  auto points = manager.GetRoutePoints();
  manager.RemoveRoutePoints();
  for (auto & point : points)
  {
    if (point.m_pointType == RouteMarkType::Start)
      point.m_pointType = RouteMarkType::Finish;
    else if (point.m_pointType == RouteMarkType::Finish)
      point.m_pointType = RouteMarkType::Start;
    manager.AddRoutePoint(std::move(point));
  }
  UpdatePoints();
  rebuild();
}

void Routing::rebuild()
{
  if (m_state == QLatin1String("idle"))
    return;

  auto & manager = m_framework.GetRoutingManager();
  bool hasStart = false;
  bool hasFinish = false;
  for (auto const & point : manager.GetRoutePoints())
  {
    hasStart |= point.m_pointType == RouteMarkType::Start;
    hasFinish |= point.m_pointType == RouteMarkType::Finish;
  }

  m_ready = false;
  m_missingMaps.clear();
  m_absentCountries.clear();
  if (!hasStart || !hasFinish)
  {
    m_building = false;
    m_error = hasFinish ? tr("Waiting for your location") : tr("Choose a destination");
    emit routeChanged();
    return;
  }

  m_error.clear();
  m_building = true;
  emit routeChanged();
  manager.BuildRoute();
}

void Routing::OnRouteBuilt(routing::RouterResultCode code, storage::CountriesSet const & absent)
{
  using routing::RouterResultCode;
  m_building = false;
  m_ready = code == RouterResultCode::NoError || code == RouterResultCode::HasWarnings;
  m_error = m_ready ? QString() : ErrorText(code);
  m_waitingForPosition = code == RouterResultCode::NoCurrentPosition;
  m_absentCountries = absent;
  m_missingMaps.clear();
  for (auto const & countryId : absent)
  {
    storage::NodeAttrs attrs;
    m_framework.GetStorage().GetNodeAttrs(countryId, attrs);
    m_missingMaps << QString::fromStdString(attrs.m_nodeLocalName);
  }
  emit routeChanged();
  UpdatePoints();
  UpdateProgress();
}

void Routing::start()
{
  auto & manager = m_framework.GetRoutingManager();
  if (!m_ready || manager.IsRoutingFollowing())
    return;
  if (m_framework.GetCurrentPosition())
  {
    Follow();
  }
  else
  {
    PlaceAtStart();
    m_followTimer.start();
  }
  SetState(QStringLiteral("navigating"));
  UpdateProgress();
}

void Routing::Follow()
{
  auto & manager = m_framework.GetRoutingManager();
  manager.FollowRoute();
  if (manager.GetRouter() == routing::RouterType::Vehicle)
    m_framework.SwitchToUsingVehicleStyle(true);
}

void Routing::PlaceAtStart()
{
  for (auto const & point : m_framework.GetRoutingManager().GetRoutePoints())
  {
    if (point.m_pointType != RouteMarkType::Start || point.m_isMyPosition)
      continue;
    auto const ll = mercator::ToLatLon(point.m_position);
    location::GpsInfo info;
    info.m_source = location::EUser;
    info.m_timestamp = static_cast<double>(QDateTime::currentSecsSinceEpoch());
    info.m_latitude = ll.m_lat;
    info.m_longitude = ll.m_lon;
    info.m_horizontalAccuracy = 10.0;
    m_framework.OnLocationUpdate(info);
    return;
  }
}

void Routing::cancel()
{
  m_followTimer.stop();
  m_framework.GetRoutingManager().CloseRouting(true);
  m_framework.SwitchToUsingVehicleStyle(false);
  m_building = false;
  m_ready = false;
  m_error.clear();
  m_missingMaps.clear();
  m_finished = false;
  m_waitingForPosition = false;
  setPickTarget({});
  SetState(QStringLiteral("idle"));
  emit routeChanged();
}

void Routing::downloadMissingMaps()
{
  for (auto const & countryId : m_absentCountries)
    m_framework.GetStorage().DownloadNode(countryId);
}

void Routing::UpdatePoints()
{
  m_fromName.clear();
  m_toName.clear();
  m_fromIsMyPosition = false;
  for (auto const & point : m_framework.GetRoutingManager().GetRoutePoints())
  {
    QString const name = point.m_isMyPosition ? tr("My position") : QString::fromStdString(point.m_title);
    if (point.m_pointType == RouteMarkType::Start)
    {
      m_fromName = name;
      m_fromIsMyPosition = point.m_isMyPosition;
    }
    else if (point.m_pointType == RouteMarkType::Finish)
      m_toName = name.isEmpty() ? tr("Dropped pin") : name;
  }
  emit pointsChanged();
}

void Routing::UpdateProgress()
{
  if (m_waitingForPosition && !m_building && m_framework.GetCurrentPosition())
  {
    m_waitingForPosition = false;
    rebuild();
    return;
  }

  auto const & manager = m_framework.GetRoutingManager();
  if (!manager.IsRouteBuilt())
    return;

  routing::FollowingInfo info;
  manager.GetRouteFollowingInfo(info);
  if (!info.IsValid())
    return;

  bool const pedestrian = manager.GetRouter() == routing::RouterType::Pedestrian ||
                          manager.GetRouter() == routing::RouterType::Transit;

  m_totalTime = FormatDuration(info.m_time);
  m_totalDistance = QString::fromStdString(info.m_distToTarget.ToString());
  m_arrival = FormatClockIn(info.m_time);
  m_completion = info.m_completionPercent / 100.0;
  m_exitNumber = static_cast<int>(info.m_exitNum);
  m_turnGlyph = pedestrian ? PedestrianGlyph(info.m_pedestrianTurn) : CarGlyph(info.m_turn);
  m_turnText = pedestrian ? PedestrianText(info.m_pedestrianTurn) : CarText(info.m_turn, m_exitNumber);
  m_distanceToTurn = QString::fromStdString(info.m_distToTurn.ToString());
  m_nextStreet = QString::fromStdString(info.m_nextStreetName);
  m_currentStreet = QString::fromStdString(info.m_currentStreetName);

  bool const thenClose = info.m_distToTurn.To(platform::Distance::Units::Meters).GetDistance() < kThenDistanceMeters;
  m_thenGlyph = !pedestrian && thenClose && info.m_nextTurn != CarDirection::None ? CarGlyph(info.m_nextTurn) : QString();

  m_speedLimit.clear();
  if (info.m_speedLimitMps > 0)
  {
    double const kmph = info.m_speedLimitMps * 3.6;
    m_speedLimit = measurement_utils::GetMeasurementUnits() == measurement_utils::Units::Imperial
                     ? QString::number(std::lround(measurement_utils::KmphToMiph(kmph)))
                     : QString::number(std::lround(kmph));
  }

  m_finished = info.m_routingSessionState == routing::SessionState::RouteFinished;
  emit progressChanged();
}
