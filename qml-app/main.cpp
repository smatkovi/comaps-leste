#include "location.hpp"
#include "map_controller.hpp"
#include "map_engine.hpp"
#include "map_search.hpp"
#include "place_page.hpp"
#include "routing.hpp"
#include "settings.hpp"

#include "qt/qt_common/helpers.hpp"

#include "map/framework.hpp"

#include "platform/platform.hpp"

#include "base/logging.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
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

  QGuiApplication app(argc, argv);
  QGuiApplication::setApplicationName(QStringLiteral("comaps"));
  QGuiApplication::setDesktopFileName(QStringLiteral("comaps"));
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

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app, [](QList<QQmlError> const & warnings) {
      for (auto const & warning : warnings)
        LOG(LWARNING, ("QML:", warning.toString().toStdString()));
    });
    engine.loadFromModule("Maps", "Main");
    if (engine.rootObjects().isEmpty())
      return EXIT_FAILURE;

    returnCode = QGuiApplication::exec();
  }

  LOG_SHORT(LINFO, ("Finished with code", returnCode));
  return returnCode;
}
