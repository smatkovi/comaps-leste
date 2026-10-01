#pragma once

#include <QWidget>

// About Maps, shown in a dialog from the window menu.
class AboutWidget : public QWidget
{
  Q_OBJECT

public:
  AboutWidget(QString const & version, QWidget * parent = nullptr);
};
