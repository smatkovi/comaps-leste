#include "downloader_window.hpp"

#include "map_controller.hpp"
#include "window_manager.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>

#include <cmath>

namespace
{
QString StatusText(QModelIndex const & source)
{
  auto const status = source.data(CountryTreeModel::StatusRole).toString();
  auto const size = source.data(CountryTreeModel::SizeRole).toString();
  if (status == QLatin1String("downloading"))
    return QObject::tr("Downloading %1%")
        .arg(std::lround(source.data(CountryTreeModel::ProgressRole).toDouble() * 100));
  if (status == QLatin1String("queued"))
    return QObject::tr("Waiting to download");
  if (status == QLatin1String("error"))
    return QObject::tr("Download failed");
  if (status == QLatin1String("outdated"))
    return QObject::tr("Update available · %1").arg(size);
  return source.data(CountryTreeModel::DetailRole).toString();
}
}  // namespace

DownloaderWindow::DownloaderWindow(WindowManager & manager, QString const & parentId, QWidget * parent)
  : PageWindow(tr("Map downloads"), parent)
  , m_manager(manager)
{
  m_model.setParentId(parentId);
  if (!parentId.isEmpty())
    setWindowTitle(m_model.parentName());

  auto * central = new QWidget(this);
  auto * layout = new QVBoxLayout(central);

  auto * filter = new QLineEdit(central);
  filter->setPlaceholderText(tr("Search maps"));
  filter->setClearButtonEnabled(true);
  layout->addWidget(filter);

  m_updates = new QLabel(central);
  m_updates->setWordWrap(true);
  layout->addWidget(m_updates);

  m_list = new QListView(central);
  auto * proxy = new DisplayProxy([](QModelIndex const & source)
  {
    auto name = source.data(CountryTreeModel::NameRole).toString();
    if (source.data(CountryTreeModel::HasChildrenRole).toBool())
      name += QStringLiteral(" ›");
    return TwoLines(name, StatusText(source));
  }, &m_model);
  proxy->setSourceModel(&m_model);
  m_list->setModel(proxy);
  m_list->setContextMenuPolicy(Qt::CustomContextMenu);
  layout->addWidget(m_list, 1);
  setCentralWidget(central);

  connect(filter, &QLineEdit::textChanged, &m_model, &CountryTreeModel::setFilter);
  connect(m_list, &QListView::clicked, this, &DownloaderWindow::Activate);
  connect(m_list, &QListView::customContextMenuRequested, this,
          [this](QPoint const & pos) { ShowActions(m_list->indexAt(pos)); });

  if (parentId.isEmpty())
  {
    m_updateAll = menuBar()->addAction(tr("Update all"), &m_model, &CountryTreeModel::updateAll);
    connect(&m_model, &CountryTreeModel::updatesChanged, this, &DownloaderWindow::UpdateUpdates);
  }
  UpdateUpdates();
}

void DownloaderWindow::Activate(QModelIndex const & index)
{
  if (index.data(CountryTreeModel::HasChildrenRole).toBool())
    m_manager.OpenDownloader(index.data(CountryTreeModel::IdRole).toString(), this);
  else
    ShowActions(index);
}

void DownloaderWindow::ShowActions(QModelIndex const & index)
{
  if (!index.isValid())
    return;
  auto const id = index.data(CountryTreeModel::IdRole).toString();
  auto const name = index.data(CountryTreeModel::NameRole).toString();
  auto const status = index.data(CountryTreeModel::StatusRole).toString();
  auto const size = index.data(CountryTreeModel::SizeRole).toString();
  bool const busy = status == QLatin1String("downloading") || status == QLatin1String("queued");
  bool const local = status == QLatin1String("onDisk") || status == QLatin1String("outdated") ||
                     status == QLatin1String("partly");

  QMenu menu(this);
  menu.setTitle(name);
  if (status == QLatin1String("notDownloaded") || status == QLatin1String("partly"))
    menu.addAction(tr("Download (%1)").arg(size), this, [this, id] { m_model.download(id); });
  if (status == QLatin1String("outdated"))
    menu.addAction(tr("Update"), this, [this, id] { m_model.update(id); });
  if (status == QLatin1String("error"))
    menu.addAction(tr("Retry download"), this, [this, id] { m_model.retry(id); });
  if (busy || status == QLatin1String("error"))
    menu.addAction(tr("Cancel download"), this, [this, id] { m_model.cancel(id); });
  menu.addAction(tr("Show on map"), this, [this, id]
  {
    m_manager.GetMapController().showCountry(id);
    m_manager.backToMap();
  });
  if (local && !busy)
    menu.addAction(tr("Delete"), this, [this, id, name]
    {
      if (QMessageBox::question(this, tr("Delete %1?").arg(name),
                                tr("The map will have to be downloaded again to use it offline.")) == QMessageBox::Yes)
        m_model.remove(id);
    });
  menu.exec(QCursor::pos());
}

void DownloaderWindow::UpdateUpdates()
{
  int const count = m_updateAll ? m_model.updateCount() : 0;
  m_updates->setVisible(count > 0);
  m_updates->setText(tr("%n map(s) can be updated (%1)", "", count).arg(m_model.updateSize()));
  if (m_updateAll)
    m_updateAll->setVisible(count > 0);
}
