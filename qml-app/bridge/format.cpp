#include "format.hpp"

#include "indexer/categories_holder.hpp"

#include <QGuiApplication>
#include <QInputMethod>
#include <QLocale>
#include <QTime>

std::string InputLocale()
{
  QString locale = QGuiApplication::inputMethod()->locale().name();
  locale.replace('_', '-');
  auto result = locale.toStdString();
  if (CategoriesHolder::MapLocaleToInteger(result) < 0)
    result = "en";
  return result;
}

QString FormatSize(uint64_t bytes)
{
  double constexpr kMb = 1024.0 * 1024.0;
  if (bytes >= 1024 * kMb)
    return QStringLiteral("%1 GB").arg(bytes / (1024 * kMb), 0, 'f', 1);
  if (bytes >= kMb)
    return QStringLiteral("%1 MB").arg(qRound64(bytes / kMb));
  return QStringLiteral("%1 kB").arg(qMax<qint64>(1, qRound64(bytes / 1024.0)));
}

QString FormatDuration(int seconds)
{
  int const minutes = qMax(1, (seconds + 30) / 60);
  if (minutes < 60)
    return QObject::tr("%1 min").arg(minutes);
  return QObject::tr("%1 h %2 min").arg(minutes / 60).arg(minutes % 60);
}

QString FormatClockIn(int seconds)
{
  return QLocale().toString(QTime::currentTime().addSecs(seconds), QLocale::ShortFormat);
}

QString StatusName(storage::NodeStatus status)
{
  using storage::NodeStatus;
  switch (status)
  {
  case NodeStatus::Downloading:
  case NodeStatus::Applying: return QStringLiteral("downloading");
  case NodeStatus::InQueue: return QStringLiteral("queued");
  case NodeStatus::Error: return QStringLiteral("error");
  case NodeStatus::OnDiskOutOfDate: return QStringLiteral("outdated");
  case NodeStatus::OnDisk: return QStringLiteral("onDisk");
  case NodeStatus::NotDownloaded: return QStringLiteral("notDownloaded");
  case NodeStatus::Partly: return QStringLiteral("partly");
  case NodeStatus::Undefined: break;
  }
  return {};
}
