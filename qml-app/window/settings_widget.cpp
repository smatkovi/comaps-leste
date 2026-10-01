#include "settings_widget.hpp"

#include "map_controller.hpp"
#include "settings.hpp"

#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include <cmath>
#include <functional>
#include <utility>
#include <vector>

#if defined(MAEMO)
#include <QMaemo5ListPickSelector>
#include <QMaemo5ValueButton>
#include <QStandardItemModel>
#else
#include <QComboBox>
#include <QHBoxLayout>
#endif

namespace
{
struct Choice
{
  QString m_label;
  QVariant m_value;
};

int IndexOf(std::vector<Choice> const & choices, QVariant const & value)
{
  for (size_t i = 0; i < choices.size(); ++i)
  {
    auto const & choice = choices[i].m_value;
    bool const same = choice.typeId() == QMetaType::Double ? std::abs(choice.toDouble() - value.toDouble()) < 1e-6
                                                            : choice == value;
    if (same)
      return static_cast<int>(i);
  }
  return 0;
}

// One of several values: a Maemo value button with a list picker, a combo box elsewhere.
QWidget * ChoiceButton(QString const & title, std::vector<Choice> choices, QVariant const & current,
                       std::function<void(QVariant const &)> onChosen, QWidget * parent)
{
  int const currentIndex = IndexOf(choices, current);
#if defined(MAEMO)
  auto * button = new QMaemo5ValueButton(title, parent);
  button->setValueLayout(QMaemo5ValueButton::ValueBesideText);
  auto * model = new QStandardItemModel(button);
  for (auto const & choice : choices)
    model->appendRow(new QStandardItem(choice.m_label));
  auto * picker = new QMaemo5ListPickSelector(button);
  picker->setModel(model);
  picker->setCurrentIndex(currentIndex);
  button->setPickSelector(picker);
  QObject::connect(picker, &QMaemo5ListPickSelector::selected, button,
                   [picker, choices = std::move(choices), onChosen = std::move(onChosen)](QString const &)
  {
    int const index = picker->currentIndex();
    if (index >= 0 && index < static_cast<int>(choices.size()))
      onChosen(choices[static_cast<size_t>(index)].m_value);
  });
  return button;
#else
  auto * row = new QWidget(parent);
  auto * layout = new QHBoxLayout(row);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(new QLabel(title, row), 1);
  auto * combo = new QComboBox(row);
  for (auto const & choice : choices)
    combo->addItem(choice.m_label);
  combo->setCurrentIndex(currentIndex);
  layout->addWidget(combo);
  QObject::connect(combo, &QComboBox::activated, row,
                   [choices = std::move(choices), onChosen = std::move(onChosen)](int index)
  {
    if (index >= 0 && index < static_cast<int>(choices.size()))
      onChosen(choices[static_cast<size_t>(index)].m_value);
  });
  return row;
#endif
}

QCheckBox * Toggle(QString const & text, bool checked, std::function<void(bool)> onToggled, QWidget * parent)
{
  auto * box = new QCheckBox(text, parent);
  box->setChecked(checked);
  QObject::connect(box, &QCheckBox::toggled, box, std::move(onToggled));
  return box;
}

QLabel * Section(QString const & text, QWidget * parent)
{
  auto * label = new QLabel(text, parent);
  auto font = label->font();
  font.setBold(true);
  label->setFont(font);
  return label;
}

std::vector<Choice> SpeedChoices()
{
  // 0: 1.6× up to zoom 16, 1.3× closer in.
  return {{QObject::tr("Dynamic"), 0.0}, {QObject::tr("1×"), 1.0}, {QObject::tr("1.3×"), 1.3},
          {QObject::tr("1.6×"), 1.6},    {QObject::tr("2×"), 2.0}};
}
}  // namespace

SettingsWidget::SettingsWidget(Settings & settings, MapController & mapController, QWidget * parent) : QWidget(parent)
{
  auto * outer = new QVBoxLayout(this);
  outer->setContentsMargins(0, 0, 0, 0);
  auto * scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  outer->addWidget(scroll);

  auto * content = new QWidget(scroll);
  auto * layout = new QVBoxLayout(content);
  scroll->setWidget(content);

  auto * s = &settings;
  auto * mc = &mapController;

  layout->addWidget(Section(tr("General"), content));
  layout->addWidget(ChoiceButton(
      tr("Theme"),
      {{tr("Dark"), QStringLiteral("dark")}, {tr("Light"), QStringLiteral("light")},
       {tr("Follow system"), QStringLiteral("system")}},
      settings.theme(), [s](QVariant const & v) { s->setTheme(v.toString()); }, content));
  layout->addWidget(ChoiceButton(tr("Units"), {{tr("Metric"), 0}, {tr("Imperial"), 1}}, settings.units(),
                                 [s](QVariant const & v) { s->setUnits(v.toInt()); }, content));
  layout->addWidget(ChoiceButton(tr("Pinch zoom speed"), SpeedChoices(), settings.pinchSpeed(),
                                 [s](QVariant const & v) { s->setPinchSpeed(v.toDouble()); }, content));
  layout->addWidget(ChoiceButton(tr("Scroll speed"), SpeedChoices(), settings.scrollSpeed(),
                                 [s](QVariant const & v) { s->setScrollSpeed(v.toDouble()); }, content));

  layout->addWidget(Section(tr("Map"), content));
  layout->addWidget(ChoiceButton(tr("Map label size"), SpeedChoices(), settings.labelScale(),
                                 [s](QVariant const & v) { s->setLabelScale(v.toDouble()); }, content));
  layout->addWidget(ChoiceButton(tr("Map icon size"), {{tr("1×"), 1.0}, {tr("1.25×"), 1.25}, {tr("1.5×"), 1.5}},
                                 settings.iconScale(), [s](QVariant const & v) { s->setIconScale(v.toDouble()); },
                                 content));
  layout->addWidget(ChoiceButton(
      tr("Map language"),
      {{tr("Automatic"), QStringLiteral("auto")}, {tr("Local names"), QStringLiteral("local")},
       {tr("English"), QStringLiteral("en")}},
      settings.mapLanguage(), [s](QVariant const & v) { s->setMapLanguage(v.toString()); }, content));
  layout->addWidget(Toggle(tr("3D buildings"), mapController.buildings3d(),
                           [mc](bool on) { mc->setBuildings3d(on); }, content));
  layout->addWidget(Toggle(tr("Perspective view (tilt the map when zoomed in)"), settings.perspective(),
                           [s](bool on) { s->setPerspective(on); }, content));
  layout->addWidget(Toggle(tr("Bookmark names"), settings.bookmarkLabels(),
                           [s](bool on) { s->setBookmarkLabels(on); }, content));
  layout->addWidget(Toggle(tr("Transliterate names (Latin script)"), settings.transliteration(),
                           [s](bool on) { s->setTransliteration(on); }, content));

  layout->addWidget(Section(tr("Navigation"), content));
  layout->addWidget(Toggle(tr("Tilt while navigating"), settings.navigationPerspective(),
                           [s](bool on) { s->setNavigationPerspective(on); }, content));
  layout->addWidget(Toggle(tr("Auto zoom before turns"), settings.autoZoom(),
                           [s](bool on) { s->setAutoZoom(on); }, content));

  layout->addStretch(1);
}
