#pragma once

#include "platform/location_service/location_service.hpp"

#include <QTcpSocket>
#include <QTimer>

#include <functional>
#include <memory>

/// Positions from gpsd (https://gpsd.io) over its JSON protocol. Phones whose GNSS feeds gpsd
/// (the F(x)tec Pro1X under Maemo Leste or FreeBSD: modem -> pro1x-gnss -> gpsd) run no geoclue.
/// As long as gpsd has never answered, a refused connection hands over to the fallback service
/// (geoclue2), so desktops without gpsd keep working as before.
class GpsdLocationService
  : public QObject
  , public location::LocationService
  , public location::LocationObserver
{
  Q_OBJECT

public:
  using FallbackFactory = std::function<std::unique_ptr<location::LocationService>(location::LocationObserver &)>;

  GpsdLocationService(location::LocationObserver & observer, QString const & host, quint16 port,
                      FallbackFactory fallback);
  ~GpsdLocationService() override;

  void Start() override;
  void Stop() override;

private slots:
  void OnConnected();
  void OnReadyRead();
  void OnSocketError(QAbstractSocket::SocketError error);
  void OnDisconnected();

private:
  // The fallback's events go to our observer unchanged.
  void OnLocationError(location::TLocationError errorCode) override;
  void OnLocationUpdated(location::GpsInfo const & info) override;

  void Connect();
  void HandleReport(QByteArray const & line);

  QTcpSocket m_socket;
  QTimer m_retry;
  QString m_host;
  quint16 m_port;
  FallbackFactory m_makeFallback;
  std::unique_ptr<location::LocationService> m_fallback;
  bool m_started = false;
  // gpsd answered at least once: from then on it is the source, also while it restarts.
  bool m_answered = false;
  bool m_hadFix = false;
};

/// The service for COMAPS_GPSD: unset (localhost:2947), "host[:port]", or "off" for the fallback only.
std::unique_ptr<location::LocationService> CreateGpsdLocationService(location::LocationObserver & observer,
                                                                     GpsdLocationService::FallbackFactory fallback);
