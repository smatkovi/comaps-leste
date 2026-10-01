#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QtQml/qqmlregistration.h>

class MapController;
class PlacePage;
class QJSEngine;
class QQmlEngine;
class QWidget;
class Routing;

// Opens the page windows (search, bookmarks, map downloads, place details) and the route options,
// for the window menu and for the QML on the map, and closes them again to go back to the map.
class WindowManager : public QObject
{
  Q_OBJECT
  QML_NAMED_ELEMENT(Windows)
  QML_SINGLETON

public:
  WindowManager(MapController & mapController, PlacePage & placePage, Routing & routing);
  ~WindowManager() override;

  static WindowManager * create(QQmlEngine *, QJSEngine *);
  static WindowManager & Instance();

  void SetMainWindow(QWidget * window) { m_mainWindow = window; }

  Q_INVOKABLE void openSearch(QString const & query = {});
  Q_INVOKABLE void openBookmarks();
  Q_INVOKABLE void openDownloader(QString const & parentId = {});
  Q_INVOKABLE void openPlaceDetails();
  Q_INVOKABLE void openRouteOptions();

  // Closes every page window, showing the map.
  Q_INVOKABLE void backToMap();

  void OpenBookmarkList(QString const & categoryId, QWidget * parent);
  void OpenDownloader(QString const & parentId, QWidget * parent);

  MapController & GetMapController() { return m_mapController; }
  PlacePage & GetPlacePage() { return m_placePage; }
  Routing & GetRouting() { return m_routing; }

private:
  // The window a new page stacks on: the page on top, else the main window.
  QWidget * Top() const;
  void Show(QWidget * window);

  MapController & m_mapController;
  PlacePage & m_placePage;
  Routing & m_routing;
  QPointer<QWidget> m_mainWindow;
  QList<QPointer<QWidget>> m_pages;
};
