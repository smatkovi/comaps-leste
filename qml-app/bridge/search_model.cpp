#include "search_model.hpp"
#include "format.hpp"
#include "map_engine.hpp"
#include "map_search.hpp"

#include "map/framework.hpp"

#include "search/displayed_categories.hpp"
#include "search/mode.hpp"

#include "indexer/classificator.hpp"

#include "geometry/mercator.hpp"

#include "platform/distance.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QPointer>
#include <QVariantMap>

#include <algorithm>
#include <map>
#include <vector>

namespace
{
// A group is offered once this many results share a brand or a kind of place, and at most this many
// of each kind of group are offered, so the results stay the point of the list.
int constexpr kMinGroupCount = 2;
size_t constexpr kMaxGroupsPerKind = 2;
}  // namespace

SearchModel::SearchModel(QObject * parent) : QAbstractListModel(parent) {}

SearchModel::~SearchModel()
{
  // Only this list's search: a search shown on the map outlives the page (see MapSearch).
  AppFramework().GetSearchAPI().CancelSearch(search::Mode::Everywhere);
}

void SearchModel::setQuery(QString const & query)
{
  if (m_query == query)
    return;
  m_query = query;
  emit queryChanged();
  Run(false);
}

void SearchModel::Run(bool isCategory)
{
  auto & api = AppFramework().GetSearchAPI();
  auto const timestamp = ++m_timestamp;

  beginResetModel();
  m_results.Clear();
  endResetModel();
  emit countChanged();
  m_groups.clear();
  emit groupsChanged();

  std::string const query = m_query.toStdString();
  if (query.empty())
  {
    api.CancelSearch(search::Mode::Everywhere);
    SetSearching(false);
    return;
  }

  qDebug() << "search: start" << m_query << "locale" << QString::fromStdString(InputLocale()) << "category"
           << isCategory << "timestamp" << timestamp;
  // The engine keeps the callback and calls it from its own thread, also after the search was
  // cancelled, which is why it holds no raw pointer to this model: the page that owns the model is
  // popped as soon as a result or a group is picked, and a call queued on a destroyed model is
  // delivered into freed memory.
  search::EverywhereSearchParams params{
      query, InputLocale(), {}, isCategory,
      [guard = QPointer<SearchModel>(this), timestamp](search::Results results,
                                                       std::vector<search::ProductInfo>) {
        qDebug() << "search: results" << results.GetCount() << "end" << results.IsEndMarker() << "timestamp"
                 << timestamp;
        QMetaObject::invokeMethod(qApp, [guard, timestamp, results = std::move(results)]() mutable {
          if (guard)
            guard->OnResults(timestamp, std::move(results));
        }, Qt::QueuedConnection);
      }};
  SetSearching(api.SearchEverywhere(std::move(params)));
}

void SearchModel::OnResults(uint64_t timestamp, search::Results results)
{
  if (timestamp != m_timestamp)
  {
    qDebug() << "search: dropping stale results" << timestamp << "current" << m_timestamp;
    return;
  }
  beginResetModel();
  m_results = std::move(results);
  endResetModel();
  emit countChanged();
  RebuildGroups();
  for (auto const & group : m_groups)
    qDebug().nospace() << "search: group '" << group.m_label << "' category " << group.m_isCategory << " type "
                       << (group.m_type != 0 ? QString::fromStdString(classif().GetReadableObjectName(group.m_type))
                                             : QStringLiteral("any"))
                       << " (" << group.m_type << ")";
  emit groupsChanged();
  if (m_results.IsEndMarker())
    SetSearching(false);
}

void SearchModel::SetSearching(bool searching)
{
  if (m_searching == searching)
    return;
  m_searching = searching;
  emit searchingChanged();
}

int SearchModel::rowCount(QModelIndex const & parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(m_results.GetCount());
}

QVariant SearchModel::data(QModelIndex const & index, int role) const
{
  if (!index.isValid() || index.row() >= rowCount())
    return {};

  auto const & result = m_results[static_cast<size_t>(index.row())];
  switch (role)
  {
  case NameRole: return QString::fromStdString(result.GetString());
  case TypeRole:
    return result.GetResultType() == search::Result::Type::Feature ? QString::fromStdString(result.GetLocalizedFeatureType())
                                                                    : QString();
  case AddressRole: return QString::fromStdString(result.GetAddress());
  case DistanceRole:
  {
    if (!result.HasPoint() || result.IsSuggest())
      return QString();
    auto const position = AppFramework().GetCurrentPosition();
    if (!position)
      return QString();
    auto const ll = mercator::ToLatLon(*position);
    platform::Distance distance;
    double azimut;
    AppFramework().GetDistanceAndAzimut(result.GetFeatureCenter(), ll.m_lat, ll.m_lon, -1.0, distance, azimut);
    return QString::fromStdString(distance.ToString());
  }
  case SuggestRole: return result.IsSuggest();
  default: return {};
  }
}

QHash<int, QByteArray> SearchModel::roleNames() const
{
  return {{NameRole, "name"}, {TypeRole, "type"}, {AddressRole, "address"}, {DistanceRole, "distance"},
          {SuggestRole, "suggest"}};
}

QString SearchModel::activate(int row)
{
  if (row < 0 || row >= rowCount())
    return {};
  auto const & result = m_results[static_cast<size_t>(row)];
  qDebug() << "search: activate row" << row << QString::fromStdString(result.GetString()) << "suggest"
           << result.IsSuggest();
  if (result.IsSuggest())
    return QString::fromStdString(result.GetSuggestionString());
  // Showing one place cancels every search, the one on the map included (Framework::ShowSearchResult).
  MapSearch::Instance().clear();
  AppFramework().ShowSearchResult(result);
  return {};
}

void SearchModel::searchCategory(QString const & key)
{
  std::string const locale = InputLocale();
  std::string name;
  std::string english;
  AppFramework().GetDisplayedCategories().ForEachSynonym(
      key.toStdString(), [&](std::string const & synonym, std::string const & synonymLocale) {
        if (name.empty() && synonymLocale == locale)
          name = synonym;
        if (english.empty() && synonymLocale == "en")
          english = synonym;
      });
  if (name.empty())
    name = english;
  if (name.empty())
    return;

  m_query = QString::fromStdString(name + " ");
  emit queryChanged();
  Run(true);
}

void SearchModel::RebuildGroups()
{
  struct Bucket
  {
    int m_count = 0;
    int m_first = 0;      // position of the first result in it, to keep the results' order
    uint32_t m_type = 0;  // the classificator type the results share
    QString m_name;       // the brand's name, empty for a kind of place
    QString m_typeName;
  };

  std::map<uint32_t, Bucket> kinds;
  std::map<QString, Bucket> brands;

  int position = 0;
  for (auto const & result : m_results)
  {
    if (result.GetResultType() != search::Result::Type::Feature || !result.HasPoint())
      continue;
    QString const typeName = QString::fromStdString(result.GetLocalizedFeatureType());
    if (typeName.isEmpty())
      continue;
    uint32_t const type = result.GetFeatureType();
    ++position;

    auto const fill = [&](Bucket & bucket) {
      if (bucket.m_count == 0)
        bucket.m_first = position;
      ++bucket.m_count;
      bucket.m_type = type;
      bucket.m_typeName = typeName;
    };

    fill(kinds[type]);

    QString const name = QString::fromStdString(result.GetString()).trimmed();
    if (name.isEmpty())
      continue;
    auto & brand = brands[name.toLower() + QLatin1Char('\t') + QString::number(type)];
    fill(brand);
    brand.m_name = name;
  }

  auto const take = [](auto const & buckets, size_t limit) {
    std::vector<Bucket> result;
    for (auto const & [key, bucket] : buckets)
      if (bucket.m_count >= kMinGroupCount)
        result.push_back(bucket);
    // The biggest first; equal ones in the order the results came in, which is by rank.
    std::sort(result.begin(), result.end(), [](Bucket const & lhs, Bucket const & rhs) {
      return lhs.m_count != rhs.m_count ? lhs.m_count > rhs.m_count : lhs.m_first < rhs.m_first;
    });
    if (result.size() > limit)
      result.resize(limit);
    return result;
  };

  m_groups.clear();
  // A brand is the narrower search of the two, so it comes first.
  for (auto const & bucket : take(brands, kMaxGroupsPerKind))
    m_groups.push_back({QStringLiteral("%1 · %2").arg(bucket.m_name, bucket.m_typeName),
                        tr("Show all on the map"), QStringLiteral("storefront"), bucket.m_type,
                        false /* isCategory */});
  for (auto const & bucket : take(kinds, kMaxGroupsPerKind))
    m_groups.push_back({bucket.m_typeName, tr("Show all of this kind on the map"), QStringLiteral("category"),
                        bucket.m_type, true /* isCategory */});
}

QVariantList SearchModel::groups() const
{
  QVariantList result;
  for (auto const & group : m_groups)
    result.append(QVariantMap{{"label", group.m_label}, {"hint", group.m_hint}, {"glyph", group.m_glyph}});
  return result;
}

void SearchModel::showGroupOnMap(int index)
{
  if (index < 0 || index >= static_cast<int>(m_groups.size()))
    return;
  auto const & group = m_groups[static_cast<size_t>(index)];
  // A kind of place is every place of that type around the area the map shows, found in the map
  // data itself; a brand is the results in hand, which are all of them.
  if (group.m_isCategory)
    MapSearch::Instance().ShowType(group.m_label, group.m_type);
  else
    MapSearch::Instance().ShowResults(group.m_label, group.m_type, m_results);
}

void SearchModel::showResultsOnMap()
{
  MapSearch::Instance().ShowResults(m_query, 0 /* every kind */, m_results);
}

void SearchModel::cancel()
{
  AppFramework().GetSearchAPI().CancelSearch(search::Mode::Everywhere);
  SetSearching(false);
}
