#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class Framework;
class QJSEngine;
class QQmlEngine;

class Settings : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(int units READ units WRITE setUnits NOTIFY changed)
  Q_PROPERTY(double labelScale READ labelScale WRITE setLabelScale NOTIFY changed)
  Q_PROPERTY(double iconScale READ iconScale WRITE setIconScale NOTIFY changed)
  Q_PROPERTY(bool transliteration READ transliteration WRITE setTransliteration NOTIFY changed)
  Q_PROPERTY(bool bookmarkLabels READ bookmarkLabels WRITE setBookmarkLabels NOTIFY changed)
  Q_PROPERTY(QString mapLanguage READ mapLanguage WRITE setMapLanguage NOTIFY changed)
  Q_PROPERTY(bool perspective READ perspective WRITE setPerspective NOTIFY changed)
  Q_PROPERTY(bool navigationPerspective READ navigationPerspective WRITE setNavigationPerspective NOTIFY changed)
  Q_PROPERTY(bool autoZoom READ autoZoom WRITE setAutoZoom NOTIFY changed)
  Q_PROPERTY(double pinchSpeed READ pinchSpeed WRITE setPinchSpeed NOTIFY changed)
  Q_PROPERTY(double scrollSpeed READ scrollSpeed WRITE setScrollSpeed NOTIFY changed)
  Q_PROPERTY(QString version READ version CONSTANT)

public:
  explicit Settings(Framework & framework);
  ~Settings() override;

  static Settings * create(QQmlEngine *, QJSEngine *);

  int units() const;
  void setUnits(int units);
  double labelScale();
  void setLabelScale(double scale);
  double iconScale();
  void setIconScale(double scale);
  bool transliteration();
  void setTransliteration(bool on);
  bool bookmarkLabels() const;
  void setBookmarkLabels(bool on);
  QString mapLanguage();
  void setMapLanguage(QString const & language);
  bool perspective();
  void setPerspective(bool on);
  bool navigationPerspective();
  void setNavigationPerspective(bool on);
  bool autoZoom();
  void setAutoZoom(bool on);
  double pinchSpeed() const;
  void setPinchSpeed(double speed);
  double scrollSpeed() const;
  void setScrollSpeed(double speed);
  QString version() const;

signals:
  void changed();

private:
  Framework & m_framework;
};
