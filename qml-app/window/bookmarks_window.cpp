#include "bookmarks_window.hpp"

#include "window_manager.hpp"

#include <QInputDialog>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>

BookmarksWindow::BookmarksWindow(WindowManager & manager, QString const & categoryId, QWidget * parent)
  : PageWindow(tr("Bookmarks"), parent)
  , m_manager(manager)
{
  m_model.setCategoryId(categoryId);
  if (!categoryId.isEmpty())
    setWindowTitle(m_model.categoryName());

  auto * central = new QWidget(this);
  auto * layout = new QVBoxLayout(central);
  m_empty = new QLabel(categoryId.isEmpty() ? tr("No bookmark lists.\nSave places from the place page to find them here.")
                                            : tr("This list is empty."),
                       central);
  m_empty->setAlignment(Qt::AlignCenter);
  m_empty->setWordWrap(true);
  layout->addWidget(m_empty);

  m_list = new QListView(central);
  auto * proxy = new DisplayProxy([](QModelIndex const & source)
  {
    auto const kind = source.data(BookmarkModel::KindRole).toString();
    auto name = source.data(BookmarkModel::NameRole).toString();
    if (kind == QLatin1String("category") && !source.data(BookmarkModel::ShownRole).toBool())
      name = tr("%1 (hidden)").arg(name);
    return TwoLines(name, source.data(BookmarkModel::DetailRole).toString());
  }, &m_model);
  proxy->setSourceModel(&m_model);
  m_list->setModel(proxy);
  m_list->setContextMenuPolicy(Qt::CustomContextMenu);
  layout->addWidget(m_list, 1);
  setCentralWidget(central);

  connect(m_list, &QListView::clicked, this, &BookmarksWindow::Activate);
  connect(m_list, &QListView::customContextMenuRequested, this, &BookmarksWindow::ShowActions);
  connect(&m_model, &BookmarkModel::countChanged, this, &BookmarksWindow::UpdateEmpty);
  UpdateEmpty();

  if (categoryId.isEmpty())
    menuBar()->addAction(tr("New list"), this, &BookmarksWindow::NewList);
}

void BookmarksWindow::Activate(QModelIndex const & index)
{
  auto const id = index.data(BookmarkModel::IdRole).toString();
  auto const kind = index.data(BookmarkModel::KindRole).toString();
  if (kind == QLatin1String("category"))
  {
    m_manager.OpenBookmarkList(id, this);
    return;
  }
  m_model.show(id, kind);
  m_manager.backToMap();
}

void BookmarksWindow::ShowActions(QPoint const & pos)
{
  auto const index = m_list->indexAt(pos);
  if (!index.isValid())
    return;
  auto const id = index.data(BookmarkModel::IdRole).toString();
  auto const kind = index.data(BookmarkModel::KindRole).toString();
  auto const name = index.data(BookmarkModel::NameRole).toString();
  bool const shown = index.data(BookmarkModel::ShownRole).toBool();

  QMenu menu(this);
  menu.addAction(tr("Show on map"), this, [this, id, kind]
  {
    m_model.show(id, kind);
    m_manager.backToMap();
  });
  if (kind == QLatin1String("category"))
    menu.addAction(shown ? tr("Hide on the map") : tr("Show on the map"), this,
                   [this, id, shown] { m_model.setShown(id, !shown); });
  menu.addAction(tr("Rename"), this, [this, id, kind, name]
  {
    bool ok = false;
    auto const newName = QInputDialog::getText(this, tr("Rename"), tr("Name"), QLineEdit::Normal, name, &ok).trimmed();
    if (ok && !newName.isEmpty())
      m_model.rename(id, kind, newName);
  });
  menu.addAction(tr("Delete"), this, [this, id, kind, name]
  {
    auto const text = kind == QLatin1String("category") ? tr("All bookmarks and tracks in this list are deleted.")
                                                        : QString();
    if (QMessageBox::question(this, tr("Delete %1?").arg(name), text.isEmpty() ? tr("Delete %1?").arg(name) : text) ==
        QMessageBox::Yes)
      m_model.remove(id, kind);
  });
  menu.exec(m_list->viewport()->mapToGlobal(pos));
}

void BookmarksWindow::NewList()
{
  bool ok = false;
  auto const name = QInputDialog::getText(this, tr("New list"), tr("List name"), QLineEdit::Normal, {}, &ok).trimmed();
  if (ok && !name.isEmpty())
    m_model.createList(name);
}

void BookmarksWindow::UpdateEmpty()
{
  bool const empty = m_model.rowCount() == 0;
  m_empty->setVisible(empty);
  m_list->setVisible(!empty);
}
