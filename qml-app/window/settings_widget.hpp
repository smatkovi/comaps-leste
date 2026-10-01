#pragma once

#include <QWidget>

class MapController;
class Settings;

// The app settings, shown in a dialog from the window menu. Changes go straight to Settings and
// MapController, so the map and the QML pages follow them at once.
class SettingsWidget : public QWidget
{
  Q_OBJECT

public:
  SettingsWidget(Settings & settings, MapController & mapController, QWidget * parent = nullptr);
};
