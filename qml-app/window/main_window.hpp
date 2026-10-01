#pragma once

#include <QMainWindow>
#include <QPointer>

class MapController;
class QAction;
class QDialog;
class QMenu;
class QQuickWidget;
class Settings;

// The app window: the QML map UI in a QQuickWidget, with the window menu (the Hildon app menu on
// Maemo) and the settings and about dialogs as widgets.
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(Settings & settings, MapController & mapController, QWidget * parent = nullptr);

  // Whether the QML loaded.
  bool IsLoaded() const;

private slots:
  // The actions of the page on top (e.g. Route options while planning), after the fixed items.
  void UpdatePageActions();

private:
  void CallQml(char const * function);
  void OpenSettings();
  void OpenAbout();

  Settings & m_settings;
  MapController & m_mapController;
  QQuickWidget * m_quick = nullptr;
  QMenu * m_menu = nullptr;
  QAction * m_pageSeparator = nullptr;
  QList<QAction *> m_pageActions;
  QPointer<QDialog> m_settingsDialog;
  QPointer<QDialog> m_aboutDialog;
};
