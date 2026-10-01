#include "place_details_window.hpp"

#include "place_page.hpp"
#include "routing.hpp"
#include "window_manager.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

PlaceDetailsWindow::PlaceDetailsWindow(WindowManager & manager, QWidget * parent)
  : PageWindow(manager.GetPlacePage().title(), parent)
  , m_manager(manager)
{
  auto * scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  auto * content = new QWidget(scroll);
  auto * layout = new QVBoxLayout(content);
  scroll->setWidget(content);
  setCentralWidget(scroll);

  m_header = new QLabel(content);
  m_header->setWordWrap(true);
  layout->addWidget(m_header);

  auto * buttons = new QHBoxLayout;
  m_routeHere = new QPushButton(tr("Route here"), content);
  m_routeFrom = new QPushButton(tr("Route from here"), content);
  m_bookmark = new QPushButton(content);
  buttons->addWidget(m_routeHere);
  buttons->addWidget(m_routeFrom);
  buttons->addWidget(m_bookmark);
  layout->addLayout(buttons);

  m_details = new QWidget(content);
  m_detailsLayout = new QVBoxLayout(m_details);
  m_detailsLayout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_details);
  layout->addStretch(1);

  auto & place = manager.GetPlacePage();
  connect(m_routeHere, &QPushButton::clicked, this, [this]
  {
    m_manager.GetRouting().routeToSelection();
    m_manager.backToMap();
  });
  connect(m_routeFrom, &QPushButton::clicked, this, [this]
  {
    m_manager.GetRouting().routeFromSelection();
    m_manager.backToMap();
  });
  connect(m_bookmark, &QPushButton::clicked, &place, &PlacePage::toggleBookmark);
  connect(&place, &PlacePage::changed, this, &PlaceDetailsWindow::Update);
  Update();
}

void PlaceDetailsWindow::Update()
{
  auto & place = m_manager.GetPlacePage();
  if (!place.open())
  {
    close();
    return;
  }
  setWindowTitle(place.title());

  QStringList header{QStringLiteral("<b>%1</b>").arg(place.title().toHtmlEscaped())};
  QStringList subtitle;
  for (auto const & text : {place.subtitle(), place.distance()})
    if (!text.isEmpty())
      subtitle << text.toHtmlEscaped();
  if (!subtitle.isEmpty())
    header << subtitle.join(QStringLiteral(" · "));
  if (!place.openState().isEmpty())
    header << (place.openState() == QLatin1String("open") ? tr("Open now") : tr("Closed now")) + QStringLiteral(" · ") +
                  place.openingHours().toHtmlEscaped();
  m_header->setText(header.join(QStringLiteral("<br>")));

  bool const canRoute = !place.isMyPosition();
  m_routeHere->setVisible(canRoute);
  m_routeFrom->setVisible(canRoute);
  m_bookmark->setVisible(canRoute);
  m_bookmark->setText(place.isBookmark() ? tr("Remove bookmark") : tr("Save as bookmark"));

  while (auto * item = m_detailsLayout->takeAt(0))
  {
    delete item->widget();
    delete item;
  }
  auto const addRow = [this](QString const & label, QString const & value)
  {
    if (value.isEmpty())
      return;
    auto * row = new QLabel(QStringLiteral("<small>%1</small><br>%2").arg(label.toHtmlEscaped(), value.toHtmlEscaped()),
                            m_details);
    row->setWordWrap(true);
    row->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_detailsLayout->addWidget(row);
  };
  addRow(tr("Address"), place.address());
  addRow(tr("Opening hours"), place.openState().isEmpty() ? place.openingHours() : QString());
  addRow(tr("Phone"), place.phone());
  addRow(tr("Website"), place.website());
  addRow(tr("Email"), place.email());
  addRow(tr("Cuisine"), place.cuisine());
  addRow(tr("Operator"), place.operatorName());
  addRow(tr("Level"), place.level());
  addRow(tr("Wi-Fi"), place.wifi() ? tr("Available") : QString());
  addRow(tr("Bookmark list"), place.bookmarkList());
  addRow(tr("Coordinates"), place.coordinates());
  if (!place.description().isEmpty())
  {
    auto * description = new QLabel(place.description(), m_details);
    description->setWordWrap(true);
    m_detailsLayout->addWidget(description);
  }
}
