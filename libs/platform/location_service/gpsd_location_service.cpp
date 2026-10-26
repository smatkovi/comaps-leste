#include "platform/location_service/gpsd_location_service.hpp"

#include "platform/location.hpp"

#include "base/logging.hpp"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <ctime>

namespace
{
quint16 constexpr kDefaultPort = 2947;
int constexpr kRetryMs = 3000;

// A TPV report (time-position-velocity) with a 2D or 3D fix as GpsInfo; false without a fix.
bool GpsInfoFromTpv(QJsonObject const & tpv, location::GpsInfo & info)
{
  if (tpv.value("mode").toInt() < 2 || !tpv.contains("lat") || !tpv.contains("lon"))
    return false;

  info.m_source = location::EGpsd;
  info.m_latitude = tpv.value("lat").toDouble();
  info.m_longitude = tpv.value("lon").toDouble();

  // The fix's own time (UTC); the position filter drops fixes older than five minutes.
  QDateTime const time = QDateTime::fromString(tpv.value("time").toString(), Qt::ISODateWithMs);
  info.m_timestamp = time.isValid() ? time.toMSecsSinceEpoch() / 1000.0 : static_cast<double>(std::time(nullptr));

  // eph since gpsd 3.20, before that the larger of the two axes' errors.
  if (tpv.contains("eph"))
    info.m_horizontalAccuracy = tpv.value("eph").toDouble();
  else if (tpv.contains("epx") || tpv.contains("epy"))
    info.m_horizontalAccuracy = std::max(tpv.value("epx").toDouble(), tpv.value("epy").toDouble());
  else
    info.m_horizontalAccuracy = tpv.value("mode").toInt() == 3 ? 15.0 : 50.0;

  if (tpv.value("mode").toInt() == 3)
  {
    // altMSL since gpsd 3.20, alt before.
    if (tpv.contains("altMSL"))
      info.m_altitude = tpv.value("altMSL").toDouble();
    else if (tpv.contains("alt"))
      info.m_altitude = tpv.value("alt").toDouble();
    if (tpv.contains("epv"))
      info.m_verticalAccuracy = tpv.value("epv").toDouble();
  }

  if (tpv.contains("speed"))
    info.m_speed = tpv.value("speed").toDouble();
  if (tpv.contains("track"))
    info.m_bearing = tpv.value("track").toDouble();
  return true;
}
}  // namespace

GpsdLocationService::GpsdLocationService(location::LocationObserver & observer, QString const & host,
                                         quint16 port, FallbackFactory fallback)
  : LocationService(observer)
  , m_host(host)
  , m_port(port)
  , m_makeFallback(std::move(fallback))
{
  m_retry.setSingleShot(true);
  m_retry.setInterval(kRetryMs);
  connect(&m_retry, &QTimer::timeout, this, &GpsdLocationService::Connect);
  connect(&m_socket, &QTcpSocket::connected, this, &GpsdLocationService::OnConnected);
  connect(&m_socket, &QTcpSocket::readyRead, this, &GpsdLocationService::OnReadyRead);
  connect(&m_socket, &QTcpSocket::errorOccurred, this, &GpsdLocationService::OnSocketError);
  connect(&m_socket, &QTcpSocket::disconnected, this, &GpsdLocationService::OnDisconnected);
}

GpsdLocationService::~GpsdLocationService()
{
  m_socket.disconnect(this);
  m_socket.abort();
}

void GpsdLocationService::Start()
{
  m_started = true;
  if (m_fallback)
  {
    m_fallback->Start();
    return;
  }
  if (m_socket.state() == QAbstractSocket::UnconnectedState)
    Connect();
  else if (m_socket.state() == QAbstractSocket::ConnectedState)
    OnConnected();
}

void GpsdLocationService::Stop()
{
  m_started = false;
  m_retry.stop();
  if (m_fallback)
  {
    m_fallback->Stop();
    return;
  }
  // gpsd powers the receiver down when no client watches any more.
  if (m_socket.state() == QAbstractSocket::ConnectedState)
    m_socket.write("?WATCH={\"enable\":false};\n");
  m_socket.disconnectFromHost();
  m_hadFix = false;
}

void GpsdLocationService::Connect()
{
  if (m_started && m_socket.state() == QAbstractSocket::UnconnectedState)
    m_socket.connectToHost(m_host, m_port);
}

void GpsdLocationService::OnConnected()
{
  LOG(LINFO, ("Connected to gpsd at", m_host.toStdString(), m_port));
  m_socket.write("?WATCH={\"enable\":true,\"json\":true};\n");
}

void GpsdLocationService::OnReadyRead()
{
  while (m_socket.canReadLine())
    HandleReport(m_socket.readLine());
}

void GpsdLocationService::HandleReport(QByteArray const & line)
{
  QJsonParseError error;
  QJsonObject const report = QJsonDocument::fromJson(line, &error).object();
  if (error.error != QJsonParseError::NoError)
    return;

  QString const cls = report.value("class").toString();
  if (cls == "VERSION")
  {
    m_answered = true;
    LOG(LINFO, ("gpsd", report.value("release").toString().toStdString()));
  }
  else if (cls == "TPV")
  {
    location::GpsInfo info;
    if (GpsInfoFromTpv(report, info))
    {
      m_hadFix = true;
      m_observer.OnLocationUpdated(info);
    }
  }
}

void GpsdLocationService::OnSocketError(QAbstractSocket::SocketError error)
{
  bool const unreachable =
      error == QAbstractSocket::ConnectionRefusedError || error == QAbstractSocket::HostNotFoundError;
  if (unreachable && !m_answered && m_makeFallback)
  {
    LOG(LINFO, ("No gpsd at", m_host.toStdString(), m_port, "- using the fallback location service"));
    m_fallback = m_makeFallback(*this);
    m_makeFallback = nullptr;
    if (m_fallback && m_started)
      m_fallback->Start();
    return;
  }
  LOG(LWARNING, ("gpsd:", m_socket.errorString().toStdString()));
  if (m_socket.state() == QAbstractSocket::UnconnectedState)
    OnDisconnected();
}

void GpsdLocationService::OnDisconnected()
{
  if (m_fallback || !m_started)
    return;
  if (m_hadFix)
  {
    m_hadFix = false;
    m_observer.OnLocationError(location::TLocationError::EGPSIsOff);
  }
  m_retry.start();
}

void GpsdLocationService::OnLocationError(location::TLocationError errorCode)
{
  m_observer.OnLocationError(errorCode);
}

void GpsdLocationService::OnLocationUpdated(location::GpsInfo const & info)
{
  m_observer.OnLocationUpdated(info);
}

std::unique_ptr<location::LocationService> CreateGpsdLocationService(location::LocationObserver & observer,
                                                                     GpsdLocationService::FallbackFactory fallback)
{
  QString const setting = qEnvironmentVariable("COMAPS_GPSD");
  if (setting == "off")
    return fallback ? fallback(observer) : nullptr;

  QString host = "localhost";
  quint16 port = kDefaultPort;
  if (!setting.isEmpty())
  {
    qsizetype const colon = setting.lastIndexOf(':');
    host = colon < 0 ? setting : setting.left(colon);
    if (colon >= 0)
      port = static_cast<quint16>(setting.mid(colon + 1).toUInt());
  }
  return std::make_unique<GpsdLocationService>(observer, host, port, std::move(fallback));
}
