#include "window_manager.hpp"

#include "bookmarks_window.hpp"
#include "downloader_window.hpp"
#include "place_details_window.hpp"
#include "routing.hpp"
#include "search_window.hpp"

#include "base/assert.hpp"

#include <QCheckBox>
#include <QDialog>
#include <QJSEngine>
#include <QVBoxLayout>

namespace
{
WindowManager * g_windowManager = nullptr;
}  // namespace

WindowManager::WindowManager(MapController & mapController, PlacePage & placePage, Routing & routing)
  : m_mapController(mapController)
  , m_placePage(placePage)
  , m_routing(routing)
{
  g_windowManager = this;
}

WindowManager::~WindowManager()
{
  g_windowManager = nullptr;
}

WindowManager * WindowManager::create(QQmlEngine *, QJSEngine *)
{
  QJSEngine::setObjectOwnership(g_windowManager, QJSEngine::CppOwnership);
  return g_windowManager;
}

WindowManager & WindowManager::Instance()
{
  CHECK(g_windowManager, ());
  return *g_windowManager;
}

void WindowManager::openSearch(QString const & query)
{
  Show(new SearchWindow(*this, query, Top()));
}

void WindowManager::openBookmarks()
{
  OpenBookmarkList({}, Top());
}

void WindowManager::openDownloader(QString const & parentId)
{
  OpenDownloader(parentId, Top());
}

void WindowManager::openPlaceDetails()
{
  if (m_placePage.open())
    Show(new PlaceDetailsWindow(*this, Top()));
}

void WindowManager::openRouteOptions()
{
  auto * dialog = new QDialog(Top());
  dialog->setWindowTitle(tr("Route options"));
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  auto * layout = new QVBoxLayout(dialog);
  auto * routing = &m_routing;
  bool const car = m_routing.routerType() == Routing::Car;
  auto const addOption = [&](QString const & text, bool checked, void (Routing::*set)(bool))
  {
    auto * box = new QCheckBox(text, dialog);
    box->setChecked(checked);
    connect(box, &QCheckBox::toggled, dialog, [routing, set](bool on) { (routing->*set)(on); });
    layout->addWidget(box);
  };
  if (car)
  {
    addOption(tr("Avoid toll roads"), m_routing.avoidToll(), &Routing::setAvoidToll);
    addOption(tr("Avoid motorways"), m_routing.avoidMotorway(), &Routing::setAvoidMotorway);
  }
  addOption(tr("Avoid ferries"), m_routing.avoidFerry(), &Routing::setAvoidFerry);
  addOption(tr("Avoid unpaved roads"), m_routing.avoidDirty(), &Routing::setAvoidDirty);
  if (!car)
    addOption(tr("Avoid steps"), m_routing.avoidSteps(), &Routing::setAvoidSteps);
  dialog->show();
}

void WindowManager::backToMap()
{
  // Newest first, so each closes on top of the one it was opened from.
  for (auto it = m_pages.rbegin(); it != m_pages.rend(); ++it)
    if (*it)
      (*it)->close();
  m_pages.clear();
  if (m_mainWindow)
  {
    m_mainWindow->raise();
    m_mainWindow->activateWindow();
  }
}

void WindowManager::OpenBookmarkList(QString const & categoryId, QWidget * parent)
{
  Show(new BookmarksWindow(*this, categoryId, parent));
}

void WindowManager::OpenDownloader(QString const & parentId, QWidget * parent)
{
  Show(new DownloaderWindow(*this, parentId, parent));
}

QWidget * WindowManager::Top() const
{
  for (auto it = m_pages.rbegin(); it != m_pages.rend(); ++it)
    if (*it && (*it)->isVisible())
      return *it;
  return m_mainWindow;
}

void WindowManager::Show(QWidget * window)
{
  m_pages.removeAll(nullptr);
  m_pages.append(window);
  window->show();
}
