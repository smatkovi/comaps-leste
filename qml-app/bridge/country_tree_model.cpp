#include "country_tree_model.hpp"
#include "format.hpp"
#include "map_engine.hpp"

#include "map/framework.hpp"

#include "storage/storage.hpp"

#include <algorithm>
#include <unordered_set>

namespace
{
storage::Storage & GetStorage()
{
  return AppFramework().GetStorage();
}
}  // namespace

CountryTreeModel::CountryTreeModel(QObject * parent) : QAbstractListModel(parent)
{
  m_parentId = GetStorage().GetRootId();
  m_storageSlot = GetStorage().Subscribe(
      [this](storage::CountryId const & countryId) {
        QMetaObject::invokeMethod(this, [this, countryId] {
          m_progress.erase(countryId);
          RefreshAll();
          RefreshUpdates();
        }, Qt::QueuedConnection);
      },
      [this](storage::CountryId const & countryId, downloader::Progress const & progress) {
        QMetaObject::invokeMethod(this, [this, countryId, progress] {
          if (progress.m_bytesTotal <= 0)
            return;
          m_progress[countryId] = static_cast<double>(progress.m_bytesDownloaded) / progress.m_bytesTotal;
          auto const it = std::find(m_children.begin(), m_children.end(), countryId);
          if (it == m_children.end())
            return;
          auto const row = static_cast<int>(it - m_children.begin());
          emit dataChanged(index(row), index(row), {ProgressRole, StatusRole, DetailRole});
        }, Qt::QueuedConnection);
      });
  Reload();
  RefreshUpdates();
}

CountryTreeModel::~CountryTreeModel()
{
  GetStorage().Unsubscribe(m_storageSlot);
}

void CountryTreeModel::setParentId(QString const & id)
{
  auto const newId = id.isEmpty() ? GetStorage().GetRootId() : id.toStdString();
  if (newId == m_parentId)
    return;
  m_parentId = newId;
  Reload();
  emit parentIdChanged();
}

void CountryTreeModel::setFilter(QString const & filter)
{
  auto const trimmed = filter.trimmed();
  if (trimmed == m_filter)
    return;
  m_filter = trimmed;
  Reload();
  emit filterChanged();
}

QString CountryTreeModel::parentName() const
{
  storage::NodeAttrs attrs;
  GetStorage().GetNodeAttrs(m_parentId, attrs);
  return QString::fromStdString(attrs.m_nodeLocalName);
}

void CountryTreeModel::Reload()
{
  beginResetModel();
  m_children.clear();
  auto const & storage = GetStorage();
  if (m_filter.isEmpty())
  {
    storage.GetChildren(m_parentId, m_children);
  }
  else
  {
    // A map can sit in more than one group, so the tree can list it twice.
    std::unordered_set<storage::CountryId> seen;
    storage.ForEachInSubtree(m_parentId, [&](storage::CountryId const & id, bool) {
      if (id != m_parentId && !seen.count(id) &&
          QString::fromStdString(storage.GetNodeLocalName(id)).contains(m_filter, Qt::CaseInsensitive))
      {
        seen.insert(id);
        m_children.push_back(id);
      }
    });
  }
  std::sort(m_children.begin(), m_children.end(), [&storage](auto const & a, auto const & b) {
    return QString::localeAwareCompare(QString::fromStdString(storage.GetNodeLocalName(a)),
                                       QString::fromStdString(storage.GetNodeLocalName(b))) < 0;
  });
  endResetModel();
  emit countChanged();
}

void CountryTreeModel::RefreshAll()
{
  if (!m_children.empty())
    emit dataChanged(index(0), index(static_cast<int>(m_children.size()) - 1));
}

void CountryTreeModel::RefreshUpdates()
{
  storage::Storage::UpdateInfo info;
  if (GetStorage().GetUpdateInfo(GetStorage().GetRootId(), info))
  {
    m_updateCount = static_cast<int>(info.m_numberOfMwmFilesToUpdate);
    m_updateSize = FormatSize(info.m_totalDownloadSizeInBytes);
  }
  else
  {
    m_updateCount = 0;
    m_updateSize.clear();
  }
  emit updatesChanged();
}

int CountryTreeModel::rowCount(QModelIndex const & parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(m_children.size());
}

QVariant CountryTreeModel::data(QModelIndex const & index, int role) const
{
  if (!index.isValid() || index.row() >= static_cast<int>(m_children.size()))
    return {};

  auto const & id = m_children[static_cast<size_t>(index.row())];
  auto & storage = GetStorage();
  storage::NodeAttrs attrs;
  storage.GetNodeAttrs(id, attrs);

  switch (role)
  {
  case IdRole: return QString::fromStdString(id);
  case NameRole: return QString::fromStdString(attrs.m_nodeLocalName);
  case StatusRole: return StatusName(attrs.m_status);
  case SizeRole: return FormatSize(attrs.m_mwmSize);
  case DetailRole:
  {
    if (attrs.m_mwmCounter > 1)
      return tr("%1 of %2 maps · %3")
          .arg(attrs.m_localMwmCounter)
          .arg(attrs.m_mwmCounter)
          .arg(FormatSize(attrs.m_localMwmCounter > 0 ? attrs.m_localMwmSize : attrs.m_mwmSize));
    return FormatSize(attrs.m_mwmSize);
  }
  case ProgressRole:
  {
    auto const it = m_progress.find(id);
    if (it != m_progress.end())
      return it->second;
    auto const & p = attrs.m_downloadingProgress;
    return p.m_bytesTotal > 0 ? static_cast<double>(p.m_bytesDownloaded) / p.m_bytesTotal : 0.0;
  }
  case HasChildrenRole:
  {
    storage::CountriesVec children;
    storage.GetChildren(id, children);
    return !children.empty();
  }
  default: return {};
  }
}

QHash<int, QByteArray> CountryTreeModel::roleNames() const
{
  return {{IdRole, "countryId"}, {NameRole, "name"},         {StatusRole, "status"},          {SizeRole, "size"},
          {DetailRole, "detail"}, {ProgressRole, "progress"}, {HasChildrenRole, "hasChildren"}};
}

void CountryTreeModel::download(QString const & id)
{
  GetStorage().DownloadNode(id.toStdString());
}

void CountryTreeModel::cancel(QString const & id)
{
  GetStorage().CancelDownloadNode(id.toStdString());
}

void CountryTreeModel::retry(QString const & id)
{
  GetStorage().RetryDownloadNode(id.toStdString());
}

void CountryTreeModel::update(QString const & id)
{
  GetStorage().UpdateNode(id.toStdString());
}

void CountryTreeModel::remove(QString const & id)
{
  GetStorage().DeleteNode(id.toStdString());
}

void CountryTreeModel::updateAll()
{
  GetStorage().UpdateNode(GetStorage().GetRootId());
}
