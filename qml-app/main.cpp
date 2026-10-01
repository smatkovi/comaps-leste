#include "location.hpp"
#include "main_window.hpp"
#include "map_controller.hpp"
#include "map_engine.hpp"
#include "map_search.hpp"
#include "place_page.hpp"
#include "routing.hpp"
#include "settings.hpp"
#include "window_manager.hpp"

#include "qt/qt_common/helpers.hpp"

#include "map/framework.hpp"

#include "platform/platform.hpp"

#include "base/logging.hpp"

#include <QApplication>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtGlobal>

#include <cstdlib>

int main(int argc, char * argv[])
{
  (void)::setenv("LC_NUMERIC", "C", 1);

  if (!qEnvironmentVariableIsSet("LANGUAGE") && !qEnvironmentVariableIsSet("LC_ALL") &&
      !qEnvironmentVariableIsSet("LC_MESSAGES") && !qEnvironmentVariableIsSet("LANG"))
    qputenv("LANG", "en_US.UTF-8");

  Platform & platform = GetPlatform();
  LOG(LINFO, ("Maps", platform.Version(), "built with QT:", QT_VERSION_STR, "runtime QT:", qVersion()));

  Q_INIT_RESOURCE(resources_common);

  qt::common::SetDefaultSurfaceFormat(QString());
  QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

  QApplication app(argc, argv);
  QApplication::setApplicationName(QStringLiteral("comaps"));
  QApplication::setDesktopFileName(QStringLiteral("comaps"));
  platform.SetupMeasurementSystem();

  if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE"))
    QQuickStyle::setStyle(QStringLiteral("Basic"));

  int returnCode = -1;
  {
    FrameworkParams frameworkParams;
    Framework framework(frameworkParams);
    MapEngine mapEngine(framework);
    MapController mapController(framework);
    MapSearch mapSearch(framework);
    PlacePage placePage(framework);
    Routing routingBridge(framework);
    Settings appSettings(framework);
    Location location;
    location.Start();

    WindowManager windows(mapController, placePage, routingBridge);
    MainWindow window(appSettings, mapController);
    if (!window.IsLoaded())
      return EXIT_FAILURE;
    window.show();

    returnCode = QApplication::exec();
  }

  LOG_SHORT(LINFO, ("Finished with code", returnCode));
  return returnCode;
}
