#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include <cstdint>
#include <vector>

class BookmarkModel : public QAbstractListModel
{
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(QString categoryId READ categoryId WRITE setCategoryId NOTIFY categoryIdChanged)
  Q_PROPERTY(QString categoryName READ categoryName NOTIFY countChanged)
  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
  enum Roles
  {
    IdRole = Qt::UserRole + 1,
    KindRole,
    NameRole,
    DetailRole,
    ShownRole
  };

  explicit BookmarkModel(QObject * parent = nullptr);

  int rowCount(QModelIndex const & parent = {}) const override;
  QVariant data(QModelIndex const & index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  QString categoryId() const { return m_categoryId == 0 ? QString() : QString::number(m_categoryId); }
  void setCategoryId(QString const & id);
  QString categoryName() const;

  Q_INVOKABLE void show(QString const & id, QString const & kind);
  Q_INVOKABLE void setShown(QString const & id, bool shown);
  Q_INVOKABLE void rename(QString const & id, QString const & kind, QString const & name);
  Q_INVOKABLE void remove(QString const & id, QString const & kind);
  Q_INVOKABLE void createList(QString const & name);

signals:
  void categoryIdChanged();
  void countChanged();

private:
  struct Row
  {
    uint64_t m_id;
    QString m_kind;
  };

  void Reload();

  uint64_t m_categoryId = 0;
  std::vector<Row> m_rows;
};
