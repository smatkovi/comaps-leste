#include "bookmark_model.hpp"
#include "map_engine.hpp"

#include "map/bookmark_manager.hpp"
#include "map/framework.hpp"

#include "platform/measurement_utils.hpp"

#include "geometry/mercator.hpp"

#include <algorithm>

namespace
{
std::vector<BookmarkModel *> g_bookmarkModels;

BookmarkManager & Manager()
{
  return AppFramework().GetBookmarkManager();
}
}  // namespace

BookmarkModel::BookmarkModel(QObject * parent) : QAbstractListModel(parent)
{
  if (g_bookmarkModels.empty())
  {
    Manager().SetBookmarksChangedCallback([] {
      for (auto * model : g_bookmarkModels)
        QMetaObject::invokeMethod(model, [model] { model->Reload(); }, Qt::QueuedConnection);
    });
  }
  g_bookmarkModels.push_back(this);
  connect(this, &QObject::destroyed, [this] {
    g_bookmarkModels.erase(std::remove(g_bookmarkModels.begin(), g_bookmarkModels.end(), this), g_bookmarkModels.end());
  });
  Reload();
}

void BookmarkModel::setCategoryId(QString const & id)
{
  uint64_t const value = id.toULongLong();
  if (value == m_categoryId)
    return;
  m_categoryId = value;
  emit categoryIdChanged();
  Reload();
}

QString BookmarkModel::categoryName() const
{
  return m_categoryId == 0 ? QString() : QString::fromStdString(Manager().GetCategoryName(m_categoryId));
}

void BookmarkModel::Reload()
{
  auto const & manager = Manager();
  beginResetModel();
  m_rows.clear();
  if (m_categoryId == 0)
  {
    for (auto const id : manager.GetSortedBmGroupIdList())
      m_rows.push_back({id, QStringLiteral("category")});
  }
  else if (manager.HasBmCategory(m_categoryId))
  {
    for (auto const id : manager.GetUserMarkIds(m_categoryId))
      m_rows.push_back({id, QStringLiteral("bookmark")});
    for (auto const id : manager.GetTrackIds(m_categoryId))
      m_rows.push_back({id, QStringLiteral("track")});
  }
  endResetModel();
  emit countChanged();
}

int BookmarkModel::rowCount(QModelIndex const & parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

QVariant BookmarkModel::data(QModelIndex const & index, int role) const
{
  if (!index.isValid() || index.row() >= rowCount())
    return {};

  auto const & row = m_rows[static_cast<size_t>(index.row())];
  auto const & manager = Manager();

  switch (role)
  {
  case IdRole: return QString::number(row.m_id);
  case KindRole: return row.m_kind;
  case NameRole:
  {
    if (row.m_kind == QLatin1String("category"))
      return QString::fromStdString(manager.GetCategoryName(row.m_id));
    if (row.m_kind == QLatin1String("track"))
    {
      auto const * track = manager.GetTrack(row.m_id);
      return track && !track->GetName().empty() ? QString::fromStdString(track->GetName()) : tr("Track");
    }
    auto const * bookmark = manager.GetBookmark(row.m_id);
    if (!bookmark)
      return QString();
    auto name = bookmark->GetPreferredName();
    if (name.empty())
      name = measurement_utils::FormatLatLon(mercator::YToLat(bookmark->GetPivot().y),
                                             mercator::XToLon(bookmark->GetPivot().x));
    return QString::fromStdString(name);
  }
  case DetailRole:
  {
    if (row.m_kind == QLatin1String("category"))
    {
      auto const bookmarks = manager.GetUserMarkIds(row.m_id).size();
      auto const tracks = manager.GetTrackIds(row.m_id).size();
      if (tracks == 0)
        return tr("%n bookmark(s)", "", static_cast<int>(bookmarks));
      return tr("%n bookmark(s)", "", static_cast<int>(bookmarks)) + QStringLiteral(" · ") +
             tr("%n track(s)", "", static_cast<int>(tracks));
    }
    if (row.m_kind == QLatin1String("bookmark"))
    {
      auto const * bookmark = manager.GetBookmark(row.m_id);
      return bookmark ? QString::fromStdString(bookmark->GetDescription()).left(80) : QString();
    }
    return tr("Track");
  }
  case ShownRole: return row.m_kind == QLatin1String("category") ? manager.IsVisible(row.m_id) : true;
  default: return {};
  }
}

QHash<int, QByteArray> BookmarkModel::roleNames() const
{
  return {{IdRole, "itemId"}, {KindRole, "kind"}, {NameRole, "name"}, {DetailRole, "detail"}, {ShownRole, "shown"}};
}

void BookmarkModel::show(QString const & id, QString const & kind)
{
  auto const value = id.toULongLong();
  auto & framework = AppFramework();
  if (kind == QLatin1String("category"))
    framework.ShowBookmarkCategory(value);
  else if (kind == QLatin1String("track"))
    framework.ShowTrack(value);
  else
    framework.ShowBookmark(value);
}

void BookmarkModel::setShown(QString const & id, bool shown)
{
  Manager().GetEditSession().SetIsVisible(id.toULongLong(), shown);
  Reload();
}

void BookmarkModel::rename(QString const & id, QString const & kind, QString const & name)
{
  auto const value = id.toULongLong();
  auto session = Manager().GetEditSession();
  if (kind == QLatin1String("category"))
  {
    session.SetCategoryName(value, name.toStdString());
  }
  else if (kind == QLatin1String("bookmark"))
  {
    if (auto * bookmark = session.GetBookmarkForEdit(value))
      bookmark->SetCustomName(name.toStdString());
  }
  else if (auto * track = session.GetTrackForEdit(value))
  {
    track->SetName(name.toStdString());
  }
}

void BookmarkModel::remove(QString const & id, QString const & kind)
{
  auto const value = id.toULongLong();
  auto session = Manager().GetEditSession();
  if (kind == QLatin1String("category"))
    session.DeleteBmCategory(value, true);
  else if (kind == QLatin1String("track"))
    session.DeleteTrack(value);
  else
    session.DeleteBookmark(value);
}

void BookmarkModel::createList(QString const & name)
{
  Manager().CreateBookmarkCategory(name.toStdString());
  Reload();
}
