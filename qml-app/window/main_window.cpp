#include "main_window.hpp"

#include "about_widget.hpp"
#include "settings.hpp"
#include "settings_widget.hpp"
#include "window_manager.hpp"

#include "base/logging.hpp"

#include <QAction>
#include <QDialog>
#include <QGuiApplication>
#include <QJSValue>
#include <QMenu>
#include <QMenuBar>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickWidget>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
QDialog * MakeDialog(QWidget * parent, QString const & title, QWidget * content)
{
  auto * dialog = new QDialog(parent);
  dialog->setWindowTitle(title);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  auto * layout = new QVBoxLayout(dialog);
  content->setParent(dialog);
  layout->addWidget(content);
  return dialog;
}
}  // namespace

MainWindow::MainWindow(Settings & settings, MapController & mapController, QWidget * parent)
  : QMainWindow(parent)
  , m_settings(settings)
  , m_mapController(mapController)
{
  setWindowTitle(tr("Maps"));
  WindowManager::Instance().SetMainWindow(this);
  resize(420, 820);
  setProperty("X-Maemo-StackedWindow", 1);
  setProperty("X-Maemo-Orientation", 2);

  m_quick = new QQuickWidget(this);
  m_quick->setResizeMode(QQuickWidget::SizeRootObjectToView);
  connect(m_quick->engine(), &QQmlEngine::warnings, this, [](QList<QQmlError> const & warnings)
  {
    for (auto const & warning : warnings)
      LOG(LWARNING, ("QML:", warning.toString().toStdString()));
  });
  // The Maps QML module is in the resources under /qt/qml/Maps (QQuickWidget::loadFromModule needs Qt 6.9);
  // Qt 6.5 imports from there by itself, Qt 6.4 needs the path.
  m_quick->engine()->addImportPath(QStringLiteral("qrc:/qt/qml"));
  m_quick->setSource(QUrl(QStringLiteral("qrc:/qt/qml/Maps/Main.qml")));
  for (auto const & error : m_quick->errors())
    LOG(LERROR, ("QML:", error.toString().toStdString()));
  setCentralWidget(m_quick);

  // On Maemo the platform shows the menu bar's actions as the app menu, so the bar itself stays hidden.
  m_menu = menuBar()->addMenu(tr("Maps"));
  m_menu->addAction(tr("Search"), this, [this] { CallQml("showMapSearch"); });
  m_menu->addAction(tr("Bookmarks"), this, [] { WindowManager::Instance().openBookmarks(); });
  m_menu->addAction(tr("Map downloads"), this, [] { WindowManager::Instance().openDownloader(); });
  m_menu->addSeparator();
  m_menu->addAction(tr("Settings"), this, &MainWindow::OpenSettings);
  m_menu->addAction(tr("About"), this, &MainWindow::OpenAbout);
  m_pageSeparator = m_menu->addSeparator();
  m_pageSeparator->setVisible(false);
  if (QGuiApplication::platformName().contains(QLatin1String("maemo")))
    menuBar()->hide();

  if (auto * root = m_quick->rootObject())
  {
    connect(root, SIGNAL(currentActionsChanged()), this, SLOT(UpdatePageActions()));
    UpdatePageActions();
  }
}

bool MainWindow::IsLoaded() const
{
  return m_quick->rootObject() != nullptr;
}

void MainWindow::CallQml(char const * function)
{
  if (auto * root = m_quick->rootObject())
    QMetaObject::invokeMethod(root, function);
}

void MainWindow::UpdatePageActions()
{
  for (auto * action : m_pageActions)
  {
    m_menu->removeAction(action);
    action->deleteLater();
  }
  m_pageActions.clear();

  auto * root = m_quick->rootObject();
  if (!root)
    return;
  QJSValue const actions = root->property("currentActions").value<QJSValue>();
  int const count = actions.property(QStringLiteral("length")).toInt();
  for (int i = 0; i < count; ++i)
  {
    QJSValue const item = actions.property(static_cast<quint32>(i));
    auto * action = new QAction(item.property(QStringLiteral("text")).toString(), m_menu);
    action->setCheckable(item.property(QStringLiteral("checkable")).toBool());
    action->setChecked(item.property(QStringLiteral("checked")).toBool());
    QJSValue const enabled = item.property(QStringLiteral("enabled"));
    action->setEnabled(enabled.isUndefined() || enabled.toBool());
    QJSValue const trigger = item.property(QStringLiteral("trigger"));
    connect(action, &QAction::triggered, this, [trigger]() mutable
    {
      if (trigger.isCallable())
        trigger.call();
    });
    m_menu->addAction(action);
    m_pageActions.append(action);
  }
  m_pageSeparator->setVisible(!m_pageActions.isEmpty());
}

void MainWindow::OpenSettings()
{
  if (m_settingsDialog)
  {
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
    return;
  }
  m_settingsDialog = MakeDialog(this, tr("Settings"), new SettingsWidget(m_settings, m_mapController));
  m_settingsDialog->show();
}

void MainWindow::OpenAbout()
{
  if (m_aboutDialog)
  {
    m_aboutDialog->raise();
    m_aboutDialog->activateWindow();
    return;
  }
  m_aboutDialog = MakeDialog(this, tr("About"), new AboutWidget(m_settings.version()));
  m_aboutDialog->show();
}
