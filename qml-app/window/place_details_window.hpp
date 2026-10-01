#pragma once

#include "page_window.hpp"

class QLabel;
class QPushButton;
class QVBoxLayout;
class WindowManager;

// The details of the selected place: what it is, opening hours, contacts, and routing to or from it.
class PlaceDetailsWindow : public PageWindow
{
  Q_OBJECT

public:
  PlaceDetailsWindow(WindowManager & manager, QWidget * parent);

private:
  void Update();

  WindowManager & m_manager;
  QLabel * m_header = nullptr;
  QPushButton * m_routeHere = nullptr;
  QPushButton * m_routeFrom = nullptr;
  QPushButton * m_bookmark = nullptr;
  QWidget * m_details = nullptr;
  QVBoxLayout * m_detailsLayout = nullptr;
};
