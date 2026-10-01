#pragma once

#include "search/result.hpp"

#include <QAbstractListModel>
#include <QString>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

#include <cstdint>
#include <vector>

class SearchModel : public QAbstractListModel
{
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
  Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
  // The combined searches the results suggest: one brand ("Lidl, Supermarket") or one kind of place
  // ("Supermarket") at a time, each shown on the map as a whole. {label, hint, glyph} per entry.
  Q_PROPERTY(QVariantList groups READ groups NOTIFY groupsChanged)

public:
  enum Roles
  {
    NameRole = Qt::UserRole + 1,
    TypeRole,
    AddressRole,
    DistanceRole,
    SuggestRole
  };

  explicit SearchModel(QObject * parent = nullptr);
  ~SearchModel() override;

  int rowCount(QModelIndex const & parent = {}) const override;
  QVariant data(QModelIndex const & index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  QString query() const { return m_query; }
  QVariantList groups() const;
  void setQuery(QString const & query);
  bool searching() const { return m_searching; }

  Q_INVOKABLE QString activate(int row);
  Q_INVOKABLE void searchCategory(QString const & key);
  Q_INVOKABLE void showGroupOnMap(int index);
  Q_INVOKABLE void showResultsOnMap();
  Q_INVOKABLE void cancel();

signals:
  void queryChanged();
  void searchingChanged();
  void countChanged();
  void groupsChanged();

private:
  // One combined search built from the results: the places of one brand, or every place of one
  // kind around the map's area.
  struct Group
  {
    QString m_label;
    QString m_hint;
    QString m_glyph;
    uint32_t m_type = 0;  // the only kind of place the map marks for this group
    bool m_isCategory = false;
  };

  void Run(bool isCategory);
  void OnResults(uint64_t timestamp, search::Results results);
  void SetSearching(bool searching);
  void RebuildGroups();

  QString m_query;
  search::Results m_results;
  std::vector<Group> m_groups;
  uint64_t m_timestamp = 0;
  bool m_searching = false;
};
