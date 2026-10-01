#include "page_window.hpp"

#include <QGuiApplication>
#include <QMenuBar>

PageWindow::PageWindow(QString const & title, QWidget * parent) : QMainWindow(parent)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(title);
  setProperty("X-Maemo-StackedWindow", 1);
  setProperty("X-Maemo-Orientation", 2);
  // On Maemo the menu bar's actions are the window's app menu; the bar itself stays hidden.
  if (QGuiApplication::platformName().contains(QLatin1String("maemo")))
    menuBar()->hide();
}
