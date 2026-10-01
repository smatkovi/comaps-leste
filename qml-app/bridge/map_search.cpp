#include "map_search.hpp"

#include "map/framework.hpp"

#include "search/mode.hpp"

#include "indexer/classificator.hpp"

#include "geometry/mercator.hpp"

#include <QDebug>
#include <QJSEngine>

namespace
{
MapSearch * g_mapSearch = nullptr;

double constexpr kSearchRadiusMeters = 15000.0;
size_t constexpr kMaxMarks = 200;

// One line per result for the debug log: what it is, where it is, what the engine matched.
QString ResultLine(search::Result const & result)
{
  QString line = QStringLiteral("'%1'").arg(QString::fromStdString(result.GetString()));
  if (result.GetResultType() == search::Result::Type::Feature)
  {
    line += QStringLiteral(" type %1 (%2)")
                .arg(QString::fromStdString(classif().GetReadableObjectName(result.GetFeatureType())))
                .arg(result.GetFeatureType());
    line += QStringLiteral(" %1").arg(QString::fromStdString(DebugPrint(result.GetFeatureID())));
  }
  else
  {
    line += QStringLiteral(" resultType %1").arg(static_cast<int>(result.GetResultType()));
  }
  if (result.HasPoint())
  {
    auto const ll = mercator::ToLatLon(result.GetFeatureCenter());
    line += QStringLiteral(" at %1,%2").arg(ll.m_lat, 0, 'f', 6).arg(ll.m_lon, 0, 'f', 6);
  }
  if (!result.GetAddress().empty())
    line += QStringLiteral(" address '%1'").arg(QString::fromStdString(result.GetAddress()));
  return line;
}
}  // namespace

MapSearch::MapSearch(Framework & framework) : m_framework(framework)
{
  g_mapSearch = this;
}

MapSearch::~MapSearch()
{
  g_mapSearch = nullptr;
}

MapSearch * MapSearch::create(QQmlEngine *, QJSEngine *)
{
  QJSEngine::setObjectOwnership(g_mapSearch, QJSEngine::CppOwnership);
  return g_mapSearch;
}

MapSearch & MapSearch::Instance()
{
  return *g_mapSearch;
}

void MapSearch::ShowType(QString const & label, uint32_t type)
{
  m_framework.GetSearchAPI().CancelSearch(search::Mode::Viewport);
  m_framework.SetSearchMarksType(type);

  auto const viewport = m_framework.GetCurrentViewport();
  auto area = mercator::RectByCenterXYAndSizeInMeters(viewport.Center(), 2 * kSearchRadiusMeters);
  area.Add(viewport);

  auto const results = m_framework.GetFeaturesOfTypeInRect(type, area, kMaxMarks);
  m_framework.FillSearchResultsMarks(true /* clear */, results);

  m_count = static_cast<int>(results.GetCount());
  qDebug().nospace() << "search: on map '" << label << "' marked " << m_count << " place(s) of type "
                     << QString::fromStdString(classif().GetReadableObjectName(type)) << " within "
                     << kSearchRadiusMeters / 1000.0 << " km";

  m_label = label.isEmpty() ? tr("Search results") : label;
  m_searching = false;
  emit changed();
}

void MapSearch::ShowResults(QString const & label, uint32_t type, search::Results const & results)
{
  // Drops a search still running on the map, and its marks.
  m_framework.GetSearchAPI().CancelSearch(search::Mode::Viewport);
  m_framework.SetSearchMarksType(type);
  m_framework.FillSearchResultsMarks(true /* clear */, results);

  m_count = 0;
  for (auto const & result : results)
  {
    if (result.HasPoint() && m_framework.IsSearchMarkType(result))
      ++m_count;
    else
      qDebug().nospace() << "search: on map dropped " << ResultLine(result);
  }
  qDebug().nospace() << "search: on map '" << label << "' marked " << m_count << " of " << results.GetCount()
                     << " result(s) already found";

  m_label = label.isEmpty() ? tr("Search results") : label;
  m_searching = false;
  emit changed();
}

void MapSearch::clear()
{
  // Also drops the marks, through SearchAPI's delegate.
  m_framework.GetSearchAPI().CancelSearch(search::Mode::Viewport);
  m_framework.SetSearchMarksType(0);
  m_label.clear();
  m_count = 0;
  m_searching = false;
  emit changed();
}
