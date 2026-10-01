#pragma once

#include "platform/location_service/location_service.hpp"

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include <memory>

class QJSEngine;
class QQmlEngine;

class Location : public QObject, public location::LocationObserver
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(bool hasFix READ hasFix NOTIFY changed)
  Q_PROPERTY(bool failed READ failed NOTIFY changed)
  Q_PROPERTY(double latitude READ latitude NOTIFY changed)
  Q_PROPERTY(double longitude READ longitude NOTIFY changed)
  Q_PROPERTY(double accuracy READ accuracy NOTIFY changed)
  Q_PROPERTY(double speed READ speed NOTIFY changed)
  Q_PROPERTY(QString place READ place NOTIFY placeChanged)

public:
  Location();
  ~Location() override;

  static Location * create(QQmlEngine *, QJSEngine *);

  void Start();

  bool hasFix() const { return m_hasFix; }
  bool failed() const { return m_failed; }
  double latitude() const { return m_info.m_latitude; }
  double longitude() const { return m_info.m_longitude; }
  double accuracy() const { return m_info.m_horizontalAccuracy; }
  double speed() const { return m_info.m_speed; }
  QString place() const { return m_place; }

  void OnLocationError(location::TLocationError errorCode) override;
  void OnLocationUpdated(location::GpsInfo const & info) override;

private:
  void UpdatePlace();

signals:
  void changed();
  void placeChanged();

private:
  std::unique_ptr<location::LocationService> m_service;
  location::GpsInfo m_info;
  bool m_hasFix = false;
  bool m_failed = false;
  QString m_place;
  location::GpsInfo m_placeInfo;
  bool m_hasPlaceInfo = false;
};
