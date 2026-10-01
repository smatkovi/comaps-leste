#pragma once

#include "page_window.hpp"

#include "bookmark_model.hpp"

class QLabel;
class QListView;
class WindowManager;

// The bookmark lists, or the bookmarks and tracks of one list.
class BookmarksWindow : public PageWindow
{
  Q_OBJECT

public:
  BookmarksWindow(WindowManager & manager, QString const & categoryId, QWidget * parent);

private:
  void Activate(QModelIndex const & index);
  void ShowActions(QPoint const & pos);
  void NewList();
  void UpdateEmpty();

  WindowManager & m_manager;
  BookmarkModel m_model;
  QListView * m_list = nullptr;
  QLabel * m_empty = nullptr;
};
