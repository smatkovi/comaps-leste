#pragma once

#include "storage/storage_defines.hpp"

#include <QAbstractListModel>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include <unordered_map>

class CountryTreeModel : public QAbstractListModel
{
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(QString parentId READ parentId WRITE setParentId NOTIFY parentIdChanged)
  Q_PROPERTY(QString parentName READ parentName NOTIFY parentIdChanged)
  // Non-empty: every map or group below the parent whose name contains it, instead of the children.
  Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  Q_PROPERTY(int updateCount READ updateCount NOTIFY updatesChanged)
  Q_PROPERTY(QString updateSize READ updateSize NOTIFY updatesChanged)

public:
  enum Roles
  {
    IdRole = Qt::UserRole + 1,
    NameRole,
    StatusRole,
    SizeRole,
    DetailRole,
    ProgressRole,
    HasChildrenRole
  };

  explicit CountryTreeModel(QObject * parent = nullptr);
  ~CountryTreeModel() override;

  int rowCount(QModelIndex const & parent = {}) const override;
  QVariant data(QModelIndex const & index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  QString parentId() const { return QString::fromStdString(m_parentId); }
  void setParentId(QString const & id);
  QString parentName() const;
  QString filter() const { return m_filter; }
  void setFilter(QString const & filter);
  int updateCount() const { return m_updateCount; }
  QString updateSize() const { return m_updateSize; }

  Q_INVOKABLE void download(QString const & id);
  Q_INVOKABLE void cancel(QString const & id);
  Q_INVOKABLE void retry(QString const & id);
  Q_INVOKABLE void update(QString const & id);
  Q_INVOKABLE void remove(QString const & id);
  Q_INVOKABLE void updateAll();

signals:
  void parentIdChanged();
  void filterChanged();
  void countChanged();
  void updatesChanged();

private:
  void Reload();
  void RefreshAll();
  void RefreshUpdates();

  storage::CountryId m_parentId;
  QString m_filter;
  storage::CountriesVec m_children;
  std::unordered_map<storage::CountryId, double> m_progress;
  int m_updateCount = 0;
  QString m_updateSize;
  int m_storageSlot = 0;
};
