#include "about_widget.hpp"

#include <QLabel>
#include <QVBoxLayout>

AboutWidget::AboutWidget(QString const & version, QWidget * parent) : QWidget(parent)
{
  auto * layout = new QVBoxLayout(this);

  auto * title = new QLabel(tr("Maps %1").arg(version), this);
  auto font = title->font();
  font.setBold(true);
  title->setFont(font);
  layout->addWidget(title);

  layout->addWidget(new QLabel(tr("Map data © OpenStreetMap contributors"), this));

  auto * licence = new QLabel(
      tr("Maps is built on the CoMaps engine, free and open source software licensed under the Apache License 2.0. "
         "Map data is © OpenStreetMap contributors and available under the Open Database License (ODbL). "
         "Maps works offline: downloaded maps stay on this device, and routes and searches never leave it."),
      this);
  licence->setWordWrap(true);
  layout->addWidget(licence);
  layout->addStretch(1);
}
