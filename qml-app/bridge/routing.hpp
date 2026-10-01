#pragma once

#include "routing/routing_callbacks.hpp"

#include "map/routing_mark.hpp"

#include "storage/storage_defines.hpp"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

class Framework;
class QJSEngine;
class QQmlEngine;

class Routing : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(QString state READ state NOTIFY stateChanged)
  Q_PROPERTY(bool building READ building NOTIFY routeChanged)
  Q_PROPERTY(bool ready READ ready NOTIFY routeChanged)
  Q_PROPERTY(QString error READ error NOTIFY routeChanged)
  Q_PROPERTY(QStringList missingMaps READ missingMaps NOTIFY routeChanged)
  Q_PROPERTY(int routerType READ routerType WRITE setRouterType NOTIFY routerTypeChanged)
  Q_PROPERTY(QString fromName READ fromName NOTIFY pointsChanged)
  Q_PROPERTY(QString toName READ toName NOTIFY pointsChanged)
  Q_PROPERTY(bool fromIsMyPosition READ fromIsMyPosition NOTIFY pointsChanged)
  Q_PROPERTY(QString pickTarget READ pickTarget WRITE setPickTarget NOTIFY pickTargetChanged)
  Q_PROPERTY(QString totalTime READ totalTime NOTIFY progressChanged)
  Q_PROPERTY(QString totalDistance READ totalDistance NOTIFY progressChanged)
  Q_PROPERTY(QString arrival READ arrival NOTIFY progressChanged)
  Q_PROPERTY(double completion READ completion NOTIFY progressChanged)
  Q_PROPERTY(QString turnGlyph READ turnGlyph NOTIFY progressChanged)
  Q_PROPERTY(QString turnText READ turnText NOTIFY progressChanged)
  Q_PROPERTY(QString distanceToTurn READ distanceToTurn NOTIFY progressChanged)
  Q_PROPERTY(QString nextStreet READ nextStreet NOTIFY progressChanged)
  Q_PROPERTY(QString currentStreet READ currentStreet NOTIFY progressChanged)
  Q_PROPERTY(QString thenGlyph READ thenGlyph NOTIFY progressChanged)
  Q_PROPERTY(int exitNumber READ exitNumber NOTIFY progressChanged)
  Q_PROPERTY(QString speedLimit READ speedLimit NOTIFY progressChanged)
  Q_PROPERTY(bool finished READ finished NOTIFY progressChanged)
  Q_PROPERTY(bool avoidToll READ avoidToll WRITE setAvoidToll NOTIFY optionsChanged)
  Q_PROPERTY(bool avoidMotorway READ avoidMotorway WRITE setAvoidMotorway NOTIFY optionsChanged)
  Q_PROPERTY(bool avoidFerry READ avoidFerry WRITE setAvoidFerry NOTIFY optionsChanged)
  Q_PROPERTY(bool avoidDirty READ avoidDirty WRITE setAvoidDirty NOTIFY optionsChanged)
  Q_PROPERTY(bool avoidSteps READ avoidSteps WRITE setAvoidSteps NOTIFY optionsChanged)

public:
  enum RouterKind
  {
    Car = 0,
    Walk,
    Bike,
    Transit
  };
  Q_ENUM(RouterKind)

  explicit Routing(Framework & framework);
  ~Routing() override;

  static Routing * create(QQmlEngine *, QJSEngine *);

  QString state() const { return m_state; }
  bool building() const { return m_building; }
  bool ready() const { return m_ready; }
  QString error() const { return m_error; }
  QStringList missingMaps() const { return m_missingMaps; }
  int routerType() const;
  void setRouterType(int type);
  QString fromName() const { return m_fromName; }
  QString toName() const { return m_toName; }
  bool fromIsMyPosition() const { return m_fromIsMyPosition; }
  QString pickTarget() const { return m_pickTarget; }
  void setPickTarget(QString const & target);
  QString totalTime() const { return m_totalTime; }
  QString totalDistance() const { return m_totalDistance; }
  QString arrival() const { return m_arrival; }
  double completion() const { return m_completion; }
  QString turnGlyph() const { return m_turnGlyph; }
  QString turnText() const { return m_turnText; }
  QString distanceToTurn() const { return m_distanceToTurn; }
  QString nextStreet() const { return m_nextStreet; }
  QString currentStreet() const { return m_currentStreet; }
  QString thenGlyph() const { return m_thenGlyph; }
  int exitNumber() const { return m_exitNumber; }
  QString speedLimit() const { return m_speedLimit; }
  bool finished() const { return m_finished; }

  bool avoidToll() const;
  void setAvoidToll(bool on);
  bool avoidMotorway() const;
  void setAvoidMotorway(bool on);
  bool avoidFerry() const;
  void setAvoidFerry(bool on);
  bool avoidDirty() const;
  void setAvoidDirty(bool on);
  bool avoidSteps() const;
  void setAvoidSteps(bool on);

  Q_INVOKABLE void routeToSelection();
  Q_INVOKABLE void routeFromSelection();
  Q_INVOKABLE void useSelectionForPick();
  Q_INVOKABLE void useMyPosition(bool isStart);
  Q_INVOKABLE void swap();
  Q_INVOKABLE void start();
  Q_INVOKABLE void cancel();
  Q_INVOKABLE void rebuild();
  Q_INVOKABLE void downloadMissingMaps();

signals:
  void stateChanged();
  void routeChanged();
  void routerTypeChanged();
  void pointsChanged();
  void progressChanged();
  void optionsChanged();
  void pickTargetChanged();

private:
  void OnRouteBuilt(routing::RouterResultCode code, storage::CountriesSet const & absent);
  void AddSelectionPoint(bool isStart);
  void EnsureMyPositionStart();
  void PlaceAtStart();
  void Follow();
  void AddMyPosition(RouteMarkType type);
  void UpdatePoints();
  void UpdateProgress();
  void SetState(QString const & state);
  bool HasOption(int option) const;
  void SetOption(int option, bool on);

  Framework & m_framework;
  QTimer m_timer;
  QTimer m_followTimer;
  QString m_state = QStringLiteral("idle");
  bool m_building = false;
  bool m_ready = false;
  QString m_error;
  QStringList m_missingMaps;
  storage::CountriesSet m_absentCountries;
  QString m_fromName;
  QString m_toName;
  bool m_fromIsMyPosition = false;
  QString m_pickTarget;
  bool m_waitingForPosition = false;
  QString m_totalTime;
  QString m_totalDistance;
  QString m_arrival;
  double m_completion = 0.0;
  QString m_turnGlyph;
  QString m_turnText;
  QString m_distanceToTurn;
  QString m_nextStreet;
  QString m_currentStreet;
  QString m_thenGlyph;
  int m_exitNumber = 0;
  QString m_speedLimit;
  bool m_finished = false;
};
