#include "place_page.hpp"

#include "map/framework.hpp"
#include "map/place_page_info.hpp"

#include "geometry/mercator.hpp"

#include "platform/distance.hpp"

#include "base/string_utils.hpp"

#include "3party/opening_hours/opening_hours.hpp"

#include <QDebug>
#include <QJSEngine>
#include <QStringList>
#include <QRegularExpression>

#include <ctime>

namespace
{
PlacePage * g_placePage = nullptr;

QString ToQString(std::string_view s)
{
  return QString::fromUtf8(s.data(), static_cast<qsizetype>(s.size()));
}

QString PlainText(std::string const & html)
{
  QString text = QString::fromStdString(html);
  text.replace(QRegularExpression(QStringLiteral("<[^>]*>")), QString());
  return text.simplified();
}
}  // namespace

PlacePage::PlacePage(Framework & framework) : m_framework(framework)
{
  g_placePage = this;
  auto const refresh = [this] { QMetaObject::invokeMethod(this, [this] { Refresh(); }, Qt::QueuedConnection); };
  m_framework.SetPlacePageListeners(refresh, [this] {
    QMetaObject::invokeMethod(this, [this] { Clear(); }, Qt::QueuedConnection);
  }, refresh, {});
}

PlacePage::~PlacePage()
{
  m_framework.SetPlacePageListeners({}, {}, {}, {});
  g_placePage = nullptr;
}

PlacePage * PlacePage::create(QQmlEngine *, QJSEngine *)
{
  QJSEngine::setObjectOwnership(g_placePage, QJSEngine::CppOwnership);
  return g_placePage;
}

void PlacePage::Clear()
{
  if (!m_open)
    return;
  m_open = false;
  emit changed();
}

void PlacePage::Refresh()
{
  if (!m_framework.HasPlacePageInfo())
  {
    Clear();
    return;
  }

  auto const & info = m_framework.GetCurrentPlacePageInfo();
  using feature::Metadata;

  m_title = QString::fromStdString(info.GetTitle());
  m_subtitle = QString::fromStdString(info.GetSubtitle());
  m_isMyPosition = info.IsMyPosition();
  m_isBookmark = info.IsBookmark();
  m_isTrack = info.IsTrack();
  m_bookmarkList = m_isBookmark ? QString::fromStdString(info.GetBookmarkCategoryName()) : QString();

  std::string address = info.GetSecondarySubtitle();
  // Without an address of its own (a shop mapped as a building without addr: tags, say), the nearest
  // addressed building around it, which is the address the search results show.
  if (address.empty() && !m_isTrack)
    address = m_framework.GetAddressAtPoint(info.GetMercator(), search::ReverseGeocoder::kLookupRadiusM)
                  .FormatAddress();
  m_address = QString::fromStdString(address);
  if (m_title.isEmpty())
    m_title = m_address.isEmpty() ? tr("Dropped pin") : m_address;

  m_distance.clear();
  if (auto const position = m_framework.GetCurrentPosition(); position && !m_isMyPosition)
  {
    auto const ll = mercator::ToLatLon(*position);
    platform::Distance distance;
    double azimut;
    if (m_framework.GetDistanceAndAzimut(info.GetMercator(), ll.m_lat, ll.m_lon, -1.0, distance, azimut))
      m_distance = QString::fromStdString(distance.ToString());
  }

  auto const hours = info.GetOpeningHours();
  m_openingHours = ToQString(hours);
  m_openState.clear();
  if (!hours.empty())
  {
    osmoh::OpeningHours const oh{std::string(hours)};
    if (oh.IsValid())
    {
      auto const now = std::time(nullptr);
      if (oh.IsOpen(now))
        m_openState = QStringLiteral("open");
      else if (oh.IsClosed(now))
        m_openState = QStringLiteral("closed");
    }
  }

  m_phone = ToQString(info.GetMetadata(Metadata::EType::FMD_PHONE_NUMBER));
  m_website = ToQString(info.GetMetadata(Metadata::EType::FMD_WEBSITE));
  m_email = ToQString(info.GetMetadata(Metadata::EType::FMD_EMAIL));
  m_operator = ToQString(info.GetMetadata(Metadata::EType::FMD_OPERATOR));
  m_level = ToQString(info.GetMetadata(Metadata::EType::FMD_LEVEL));
  m_cuisine = QString::fromStdString(info.FormatCuisines());
  m_wifi = info.HasWifi();

  auto const & wiki = info.GetWikiDescription();
  m_description = PlainText(wiki.empty() ? info.GetOSMDescription() : wiki);

  auto const ll = info.GetLatLon();
  m_coordinates = QString::fromStdString(strings::to_string_dac(ll.m_lat, 6) + ", " + strings::to_string_dac(ll.m_lon, 6));

  m_open = true;

  auto const & buildInfo = info.GetBuildInfo();
  auto const types = info.GetRawTypes();
  QStringList typeNames;
  for (auto const & type : types)
    typeNames << QString::fromStdString(type);
  qDebug().nospace() << "place page: title '" << m_title << "' subtitle '" << m_subtitle << "' address '" << m_address
                     << "' types [" << typeNames.join(QLatin1Char(' ')) << "]"
                     << " feature " << (info.IsFeature() ? QString::fromStdString(DebugPrint(info.GetID())) : QStringLiteral("none"))
                     << " secondarySubtitle '" << QString::fromStdString(info.GetSecondarySubtitle()) << "'"
                     << " tap: source " << static_cast<int>(buildInfo.m_source) << " match "
                     << static_cast<int>(buildInfo.m_match) << " long " << buildInfo.m_isLongTap << " mark "
                     << static_cast<qulonglong>(buildInfo.m_userMarkId) << " tappedFeature "
                     << (buildInfo.m_featureId.IsValid() ? QString::fromStdString(DebugPrint(buildInfo.m_featureId))
                                                         : QStringLiteral("none"))
                     << " at " << ll.m_lat << "," << ll.m_lon << " hiddenFeatures "
                     << m_framework.GetHiddenMapFeatures();

  emit changed();
}

void PlacePage::close()
{
  m_framework.DeactivateMapSelection();
  Clear();
}

void PlacePage::toggleBookmark()
{
  if (!m_framework.HasPlacePageInfo())
    return;

  auto const & info = m_framework.GetCurrentPlacePageInfo();
  auto & manager = m_framework.GetBookmarkManager();
  auto buildInfo = info.GetBuildInfo();

  if (info.IsBookmark())
  {
    auto const bookmarkId = info.GetBookmarkId();
    buildInfo.m_match = place_page::BuildInfo::Match::FeatureOnly;
    buildInfo.m_userMarkId = kml::kInvalidMarkId;
    manager.GetEditSession().DeleteBookmark(bookmarkId);
  }
  else
  {
    kml::BookmarkData data;
    data.m_name = info.FormatNewBookmarkName();
    data.m_point = info.GetMercator();
    data.m_color.m_predefinedColor = kml::PredefinedColor::Red;
    auto const * bookmark = manager.GetEditSession().CreateBookmark(std::move(data), manager.LastEditedBMCategory());
    buildInfo.m_match = place_page::BuildInfo::Match::UserMarkOnly;
    buildInfo.m_userMarkId = bookmark->GetId();
  }

  m_framework.UpdatePlacePageInfoForCurrentSelection(buildInfo);
  Refresh();
}
