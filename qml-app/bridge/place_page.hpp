#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class Framework;
class QJSEngine;
class QQmlEngine;

class PlacePage : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(bool open READ open NOTIFY changed)
  Q_PROPERTY(QString title READ title NOTIFY changed)
  Q_PROPERTY(QString subtitle READ subtitle NOTIFY changed)
  Q_PROPERTY(QString address READ address NOTIFY changed)
  Q_PROPERTY(QString distance READ distance NOTIFY changed)
  Q_PROPERTY(QString openingHours READ openingHours NOTIFY changed)
  Q_PROPERTY(QString openState READ openState NOTIFY changed)
  Q_PROPERTY(QString phone READ phone NOTIFY changed)
  Q_PROPERTY(QString website READ website NOTIFY changed)
  Q_PROPERTY(QString email READ email NOTIFY changed)
  Q_PROPERTY(QString cuisine READ cuisine NOTIFY changed)
  Q_PROPERTY(QString operatorName READ operatorName NOTIFY changed)
  Q_PROPERTY(QString level READ level NOTIFY changed)
  Q_PROPERTY(QString description READ description NOTIFY changed)
  Q_PROPERTY(QString coordinates READ coordinates NOTIFY changed)
  Q_PROPERTY(QString bookmarkList READ bookmarkList NOTIFY changed)
  Q_PROPERTY(bool wifi READ wifi NOTIFY changed)
  Q_PROPERTY(bool isBookmark READ isBookmark NOTIFY changed)
  Q_PROPERTY(bool isMyPosition READ isMyPosition NOTIFY changed)
  Q_PROPERTY(bool isTrack READ isTrack NOTIFY changed)

public:
  explicit PlacePage(Framework & framework);
  ~PlacePage() override;

  static PlacePage * create(QQmlEngine *, QJSEngine *);

  bool open() const { return m_open; }
  QString title() const { return m_title; }
  QString subtitle() const { return m_subtitle; }
  QString address() const { return m_address; }
  QString distance() const { return m_distance; }
  QString openingHours() const { return m_openingHours; }
  QString openState() const { return m_openState; }
  QString phone() const { return m_phone; }
  QString website() const { return m_website; }
  QString email() const { return m_email; }
  QString cuisine() const { return m_cuisine; }
  QString operatorName() const { return m_operator; }
  QString level() const { return m_level; }
  QString description() const { return m_description; }
  QString coordinates() const { return m_coordinates; }
  QString bookmarkList() const { return m_bookmarkList; }
  bool wifi() const { return m_wifi; }
  bool isBookmark() const { return m_isBookmark; }
  bool isMyPosition() const { return m_isMyPosition; }
  bool isTrack() const { return m_isTrack; }

  Q_INVOKABLE void close();
  Q_INVOKABLE void toggleBookmark();

signals:
  void changed();

private:
  void Refresh();
  void Clear();

  Framework & m_framework;
  bool m_open = false;
  QString m_title;
  QString m_subtitle;
  QString m_address;
  QString m_distance;
  QString m_openingHours;
  QString m_openState;
  QString m_phone;
  QString m_website;
  QString m_email;
  QString m_cuisine;
  QString m_operator;
  QString m_level;
  QString m_description;
  QString m_coordinates;
  QString m_bookmarkList;
  bool m_wifi = false;
  bool m_isBookmark = false;
  bool m_isMyPosition = false;
  bool m_isTrack = false;
};
