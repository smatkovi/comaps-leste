#pragma once

#include "search/result.hpp"

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class Framework;
class QJSEngine;
class QQmlEngine;

// A search shown on the map itself: every place it found is a mark, and the marks stay as the map
// is panned and zoomed. Outlives the search page, which is where a search on the map is started
// from.
class MapSearch : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(bool active READ active NOTIFY changed)
  Q_PROPERTY(QString label READ label NOTIFY changed)
  Q_PROPERTY(int count READ count NOTIFY changed)
  Q_PROPERTY(bool searching READ searching NOTIFY changed)

public:
  explicit MapSearch(Framework & framework);
  ~MapSearch() override;

  static MapSearch * create(QQmlEngine *, QJSEngine *);
  static MapSearch & Instance();

  bool active() const { return !m_label.isEmpty(); }
  QString label() const { return m_label; }
  int count() const { return m_count; }
  bool searching() const { return m_searching; }

  Q_INVOKABLE void clear();

  // Marks every place of one kind around the area the map showed when the search was started, so
  // that what is on the map does not change with the zoom level.
  void ShowType(QString const & label, uint32_t type);

  // Marks results that have already been found, instead of searching again.
  void ShowResults(QString const & label, uint32_t type, search::Results const & results);

signals:
  void changed();

private:
  Framework & m_framework;
  QString m_label;
  int m_count = 0;
  bool m_searching = false;
};
