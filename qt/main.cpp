#include "qt/mainwindow.hpp"
#include "qt/screenshoter.hpp"

#include "qt/qt_common/helpers.hpp"

#include "map/framework.hpp"

#include "platform/platform.hpp"
#include "platform/preferred_languages.hpp"
#include "platform/settings.hpp"

#include "coding/reader.hpp"

#include "base/logging.hpp"
#include "base/macros.hpp"

#include <QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

#include <sstream>

namespace
{
class InitializeFinalize
{
public:
  ~InitializeFinalize()
  {
    // optional - clean allocated data in protobuf library
    // useful when using memory and resource leak utilites
    // google::protobuf::ShutdownProtobufLibrary();
  }
};
}  // namespace

int main(int argc, char * argv[])
{
  // Our double parsing code (base/string_utils.hpp) needs dots as a floating point delimiters, not commas.
  // TODO: Refactor our doubles parsing code to use locale-independent delimiters.
  // For example, https://github.com/google/double-conversion can be used.
  // See http://dbaron.org/log/20121222-locale for more details.
  (void)::setenv("LC_NUMERIC", "C", 1);

  Platform & platform = GetPlatform();

  LOG(LINFO, ("CoMaps", platform.Version(), "built with QT:", QT_VERSION_STR, "runtime QT:", qVersion(),
              "detected CPU cores:", platform.CpuCores()));

  Q_INIT_RESOURCE(resources_common);

  InitializeFinalize mainGuard;
  UNUSED_VALUE(mainGuard);

  QApplication app(argc, argv);
  app.setDesktopFileName("comaps");
  platform.SetupMeasurementSystem();

  QApplication::setApplicationName("CoMaps");

#ifdef DEBUG
  static bool constexpr developerMode = true;
#else
  static bool constexpr developerMode = false;
#endif
  bool outvalue;
  if (!settings::Get(settings::kDeveloperMode, outvalue))
    settings::Set(settings::kDeveloperMode, developerMode);

  int returnCode = -1;
  {
    std::unique_ptr<qt::ScreenshotParams> screenshotParams;

    qt::common::SetDefaultSurfaceFormat(QApplication::platformName());

    FrameworkParams frameworkParams;
    Framework framework(frameworkParams);
    qt::MainWindow w(framework, std::move(screenshotParams), QApplication::primaryScreen()->geometry());
    w.show();
    returnCode = QApplication::exec();
  }

  LOG_SHORT(LINFO, ("Finished with code", returnCode));
  return returnCode;
}
