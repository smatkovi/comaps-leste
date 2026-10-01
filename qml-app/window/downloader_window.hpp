#pragma once

#include "page_window.hpp"

#include "country_tree_model.hpp"

class QAction;
class QLabel;
class QListView;
class WindowManager;

// Map downloads: the countries and regions under one parent, searchable.
class DownloaderWindow : public PageWindow
{
  Q_OBJECT

public:
  DownloaderWindow(WindowManager & manager, QString const & parentId, QWidget * parent);

private:
  void Activate(QModelIndex const & index);
  void ShowActions(QModelIndex const & index);
  void UpdateUpdates();

  WindowManager & m_manager;
  CountryTreeModel m_model;
  QListView * m_list = nullptr;
  QLabel * m_updates = nullptr;
  QAction * m_updateAll = nullptr;
};
