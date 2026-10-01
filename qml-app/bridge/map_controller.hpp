#pragma once

#include "storage/storage_defines.hpp"

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class Framework;
class QJSEngine;
class QQmlEngine;

class MapController : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(int myPositionMode READ myPositionMode NOTIFY myPositionModeChanged)
  Q_PROPERTY(bool dark READ dark WRITE setDark NOTIFY darkChanged)
  Q_PROPERTY(int mapMode READ mapMode WRITE setMapMode NOTIFY layersChanged)
  Q_PROPERTY(bool outdoor READ outdoor WRITE setOutdoor NOTIFY layersChanged)
  Q_PROPERTY(bool contourLines READ contourLines WRITE setContourLines NOTIFY layersChanged)
  Q_PROPERTY(bool buildings3d READ buildings3d WRITE setBuildings3d NOTIFY layersChanged)
  Q_PROPERTY(bool transitLines READ transitLines WRITE setTransitLines NOTIFY layersChanged)
  Q_PROPERTY(int hiddenFeatures READ hiddenFeatures WRITE setHiddenFeatures NOTIFY layersChanged)
  // Every group the map layers sheet switches, for its show/hide everything buttons.
  Q_PROPERTY(int allHiddenGroups READ allHiddenGroups CONSTANT)
  // Framework::RoadMode: roads (0), footpaths (1) or both (2).
  Q_PROPERTY(int roadMode READ roadMode WRITE setRoadMode NOTIFY layersChanged)
  Q_PROPERTY(QString countryId READ countryId NOTIFY countryChanged)
  Q_PROPERTY(QString countryName READ countryName NOTIFY countryChanged)
  Q_PROPERTY(QString countryStatus READ countryStatus NOTIFY countryChanged)
  Q_PROPERTY(QString countrySize READ countrySize NOTIFY countryChanged)
  Q_PROPERTY(double countryProgress READ countryProgress NOTIFY countryChanged)

public:
  enum PositionMode
  {
    PendingPosition = 0,
    NoPosition,
    NotFollow,
    Follow,
    FollowAndRotate
  };
  Q_ENUM(PositionMode)

  enum Mode
  {
    Walking = 0,
    Cycling,
    Driving,
    PublicTransport,
    DefaultMode
  };
  Q_ENUM(Mode)

  enum HiddenGroup
  {
    HideFood = 1 << 0,
    HideShops = 1 << 1,
    HideTourism = 1 << 2,
    HideServices = 1 << 3,
    HideHouseNumbers = 1 << 4,
    HideBuildings = 1 << 5,
    HideEntrances = 1 << 8,
    HideBarriers = 1 << 9,
    HideWaste = 1 << 10,
    HideElevators = 1 << 11
  };
  Q_ENUM(HiddenGroup)

  explicit MapController(Framework & framework);
  ~MapController() override;

  static MapController * create(QQmlEngine *, QJSEngine *);
  static MapController & Instance();

  int myPositionMode() const { return m_myPositionMode; }
  bool dark() const { return m_dark; }
  void setDark(bool dark);
  int mapMode();
  void setMapMode(int mode);
  bool outdoor();
  void setOutdoor(bool on);
  bool contourLines();
  void setContourLines(bool on);
  bool buildings3d();
  void setBuildings3d(bool on);
  bool transitLines();
  void setTransitLines(bool on);
  int hiddenFeatures() const;
  void setHiddenFeatures(int mask);
  int allHiddenGroups() const;
  int roadMode() const;
  void setRoadMode(int mode);

  QString countryId() const { return QString::fromStdString(m_countryId); }
  QString countryName() const { return m_countryName; }
  QString countryStatus() const { return m_countryStatus; }
  QString countrySize() const { return m_countrySize; }
  double countryProgress() const { return m_countryProgress; }

  Q_INVOKABLE void zoomIn();
  Q_INVOKABLE void zoomOut();
  Q_INVOKABLE void nextMyPositionMode();
  Q_INVOKABLE void downloadCountry();
  Q_INVOKABLE void cancelCountry();
  Q_INVOKABLE void retryCountry();
  Q_INVOKABLE void showCountry(QString const & countryId);

signals:
  void myPositionModeChanged();
  void darkChanged();
  void layersChanged();
  void countryChanged();

private:
  void OnCountryChanged(storage::CountryId const & countryId);
  void RefreshCountry();

  Framework & m_framework;
  int m_myPositionMode = PendingPosition;
  bool m_dark = false;
  storage::CountryId m_countryId;
  QString m_countryName;
  QString m_countryStatus;
  QString m_countrySize;
  double m_countryProgress = 0.0;
  int m_storageSlot = 0;
};
