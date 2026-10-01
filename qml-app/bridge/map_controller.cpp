#include "map_controller.hpp"
#include "format.hpp"

#include "map/framework.hpp"

#include "storage/storage.hpp"

#include <QJSEngine>

namespace
{
MapController * g_controller = nullptr;
}  // namespace

MapController::MapController(Framework & framework) : m_framework(framework)
{
  g_controller = this;

  m_framework.SetMyPositionModeListener([this](location::EMyPositionMode mode, bool) {
    QMetaObject::invokeMethod(this, [this, mode] {
      m_myPositionMode = static_cast<int>(mode);
      emit myPositionModeChanged();
    }, Qt::QueuedConnection);
  });

  m_framework.SetCurrentCountryChangedListener([this](storage::CountryId const & countryId) {
    QMetaObject::invokeMethod(this, [this, countryId] { OnCountryChanged(countryId); }, Qt::QueuedConnection);
  });

  m_storageSlot = m_framework.GetStorage().Subscribe(
      [this](storage::CountryId const & countryId) {
        QMetaObject::invokeMethod(this, [this, countryId] {
          if (countryId == m_countryId)
            RefreshCountry();
        }, Qt::QueuedConnection);
      },
      [this](storage::CountryId const & countryId, downloader::Progress const & progress) {
        QMetaObject::invokeMethod(this, [this, countryId, progress] {
          if (countryId != m_countryId || progress.m_bytesTotal <= 0)
            return;
          m_countryProgress = static_cast<double>(progress.m_bytesDownloaded) / progress.m_bytesTotal;
          m_countryStatus = QStringLiteral("downloading");
          emit countryChanged();
        }, Qt::QueuedConnection);
      });

  // Maps shows the default map, or public transport when that layer is on. Older versions can leave
  // another style (walking, cycling) in the settings even with the mode on Default, so the style is
  // always derived from the mode again.
  m_framework.SwitchToMapMode(
      m_framework.CurrentMapMode() == MapMode::PublicTransport ? MapMode::PublicTransport : MapMode::Default,
      true /* shouldAlwaysRefresh */);

  m_dark = m_framework.CurrentMapAppearance() == MapAppearance::Dark;
}

MapController::~MapController()
{
  m_framework.GetStorage().Unsubscribe(m_storageSlot);
  m_framework.SetMyPositionModeListener({});
  m_framework.SetCurrentCountryChangedListener({});
  g_controller = nullptr;
}

MapController * MapController::create(QQmlEngine *, QJSEngine *)
{
  QJSEngine::setObjectOwnership(g_controller, QJSEngine::CppOwnership);
  return g_controller;
}

MapController & MapController::Instance()
{
  return *g_controller;
}

void MapController::setDark(bool dark)
{
  if (m_dark == dark)
    return;
  m_dark = dark;
  m_framework.SwitchToMapAppearance(dark ? MapAppearance::Dark : MapAppearance::Light);
  emit darkChanged();
}

int MapController::mapMode()
{
  return static_cast<int>(m_framework.CurrentMapMode());
}

void MapController::setMapMode(int mode)
{
  m_framework.SwitchToMapMode(static_cast<MapMode>(mode));
  emit layersChanged();
}

bool MapController::outdoor()
{
  return m_framework.HasOutdoorLayer();
}

void MapController::setOutdoor(bool on)
{
  m_framework.SetOutdoorLayer(on);
  emit layersChanged();
}

bool MapController::contourLines()
{
  return m_framework.HasContourLinesLayer();
}

void MapController::setContourLines(bool on)
{
  m_framework.SetContourLinesLayer(on);
  emit layersChanged();
}

bool MapController::buildings3d()
{
  return m_framework.HasBuildings3d();
}

void MapController::setBuildings3d(bool on)
{
  m_framework.SetBuildings3d(on);
  emit layersChanged();
}

bool MapController::transitLines()
{
  return m_framework.PublicTransportMapModeHasTransitLines();
}

void MapController::setTransitLines(bool on)
{
  m_framework.PublicTransportMapModeSetTransitLines(on);
  emit layersChanged();
}

int MapController::hiddenFeatures() const
{
  return static_cast<int>(m_framework.GetHiddenMapFeatures());
}

void MapController::setHiddenFeatures(int mask)
{
  m_framework.SetHiddenMapFeatures(static_cast<uint32_t>(mask));
  emit layersChanged();
}

// Not the road mode's groups: the sheet leaves those to the road mode button.
int MapController::allHiddenGroups() const
{
  return HideFood | HideShops | HideTourism | HideServices | HideHouseNumbers | HideBuildings | HideEntrances |
         HideBarriers | HideWaste | HideElevators;
}

int MapController::roadMode() const
{
  return static_cast<int>(m_framework.GetRoadMode());
}

void MapController::setRoadMode(int mode)
{
  m_framework.SetRoadMode(static_cast<Framework::RoadMode>(mode));
  emit layersChanged();
}

void MapController::zoomIn()
{
  m_framework.Scale(Framework::SCALE_MAG, true);
}

void MapController::zoomOut()
{
  m_framework.Scale(Framework::SCALE_MIN, true);
}

void MapController::nextMyPositionMode()
{
  m_framework.SwitchMyPositionNextMode();
}

void MapController::downloadCountry()
{
  if (!m_countryId.empty())
    m_framework.GetStorage().DownloadNode(m_countryId);
}

void MapController::cancelCountry()
{
  if (!m_countryId.empty())
    m_framework.GetStorage().CancelDownloadNode(m_countryId);
}

void MapController::retryCountry()
{
  if (!m_countryId.empty())
    m_framework.GetStorage().RetryDownloadNode(m_countryId);
}

void MapController::showCountry(QString const & countryId)
{
  m_framework.ShowNode(countryId.toStdString());
}

void MapController::OnCountryChanged(storage::CountryId const & countryId)
{
  m_countryId = countryId;
  RefreshCountry();
}

void MapController::RefreshCountry()
{
  m_countryProgress = 0.0;
  if (m_countryId.empty())
  {
    m_countryName.clear();
    m_countryStatus.clear();
    m_countrySize.clear();
    emit countryChanged();
    return;
  }

  auto const & storage = m_framework.GetStorage();
  storage::NodeAttrs attrs;
  storage.GetNodeAttrs(m_countryId, attrs);
  m_countryName = QString::fromStdString(attrs.m_nodeLocalName);
  m_countryStatus = StatusName(attrs.m_status);
  m_countrySize = FormatSize(attrs.m_mwmSize);
  if (attrs.m_downloadingProgress.m_bytesTotal > 0)
    m_countryProgress = static_cast<double>(attrs.m_downloadingProgress.m_bytesDownloaded) /
                        attrs.m_downloadingProgress.m_bytesTotal;
  emit countryChanged();
}
