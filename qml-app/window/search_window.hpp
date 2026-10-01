#pragma once

#include "page_window.hpp"

#include "search_model.hpp"

class QLabel;
class QLineEdit;
class QListView;
class QListWidget;
class WindowManager;

// Search: categories while the field is empty, otherwise the combined searches and the results.
class SearchWindow : public PageWindow
{
  Q_OBJECT

public:
  SearchWindow(WindowManager & manager, QString const & query, QWidget * parent);

private:
  void UpdateViews();
  void UpdateGroups();
  void Activate(QModelIndex const & index);

  WindowManager & m_manager;
  SearchModel m_model;
  QLineEdit * m_field = nullptr;
  QListWidget * m_categories = nullptr;
  QListWidget * m_groups = nullptr;
  QListView * m_results = nullptr;
  QLabel * m_status = nullptr;
  QAction * m_showAll = nullptr;
};
