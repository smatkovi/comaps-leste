#include "search_window.hpp"

#include "window_manager.hpp"

#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMenuBar>
#include <QVBoxLayout>

namespace
{
struct Category
{
  char const * m_key;
  char const * m_label;
};

Category const kCategories[] = {
    {"category_eat", QT_TRANSLATE_NOOP("SearchWindow", "Places to eat")},
    {"category_food", QT_TRANSLATE_NOOP("SearchWindow", "Groceries")},
    {"category_fuel", QT_TRANSLATE_NOOP("SearchWindow", "Fuel")},
    {"category_parking", QT_TRANSLATE_NOOP("SearchWindow", "Parking")},
    {"category_transport", QT_TRANSLATE_NOOP("SearchWindow", "Transport")},
    {"category_hotel", QT_TRANSLATE_NOOP("SearchWindow", "Hotels")},
    {"category_atm", QT_TRANSLATE_NOOP("SearchWindow", "ATMs")},
    {"category_pharmacy", QT_TRANSLATE_NOOP("SearchWindow", "Pharmacies")},
    {"category_hospital", QT_TRANSLATE_NOOP("SearchWindow", "Hospitals")},
    {"category_toilet", QT_TRANSLATE_NOOP("SearchWindow", "Toilets")},
    {"category_wifi", QT_TRANSLATE_NOOP("SearchWindow", "Wi-Fi")},
};
}  // namespace

SearchWindow::SearchWindow(WindowManager & manager, QString const & query, QWidget * parent)
  : PageWindow(tr("Search"), parent)
  , m_manager(manager)
{
  auto * central = new QWidget(this);
  auto * layout = new QVBoxLayout(central);

  m_field = new QLineEdit(central);
  m_field->setPlaceholderText(tr("Search Maps"));
  m_field->setClearButtonEnabled(true);
  layout->addWidget(m_field);

  m_categories = new QListWidget(central);
  for (auto const & category : kCategories)
  {
    auto * item = new QListWidgetItem(tr(category.m_label), m_categories);
    item->setData(Qt::UserRole, QString::fromLatin1(category.m_key));
  }
  layout->addWidget(m_categories, 1);

  m_groups = new QListWidget(central);
  layout->addWidget(m_groups);

  m_status = new QLabel(central);
  m_status->setAlignment(Qt::AlignCenter);
  m_status->setWordWrap(true);
  layout->addWidget(m_status);

  m_results = new QListView(central);
  auto * proxy = new DisplayProxy([](QModelIndex const & source)
  {
    QStringList details;
    for (int role : {SearchModel::TypeRole, SearchModel::AddressRole, SearchModel::DistanceRole})
      if (auto const text = source.data(role).toString(); !text.isEmpty())
        details << text;
    return TwoLines(source.data(SearchModel::NameRole).toString(), details.join(QStringLiteral(" · ")));
  }, &m_model);
  proxy->setSourceModel(&m_model);
  m_results->setModel(proxy);
  layout->addWidget(m_results, 1);
  setCentralWidget(central);

  m_showAll = menuBar()->addAction(tr("Show results on map"), this, [this]
  {
    m_model.showResultsOnMap();
    m_manager.backToMap();
  });

  connect(m_field, &QLineEdit::textChanged, this, [this](QString const & text)
  {
    m_model.setQuery(text);
    UpdateViews();
  });
  connect(m_categories, &QListWidget::itemClicked, this, [this](QListWidgetItem * item)
  {
    m_model.searchCategory(item->data(Qt::UserRole).toString());
    m_field->setText(m_model.query());
  });
  connect(m_groups, &QListWidget::itemClicked, this, [this](QListWidgetItem * item)
  {
    m_model.showGroupOnMap(m_groups->row(item));
    m_manager.backToMap();
  });
  connect(m_results, &QListView::clicked, this, &SearchWindow::Activate);
  connect(&m_model, &SearchModel::countChanged, this, &SearchWindow::UpdateViews);
  connect(&m_model, &SearchModel::searchingChanged, this, &SearchWindow::UpdateViews);
  connect(&m_model, &SearchModel::groupsChanged, this, &SearchWindow::UpdateGroups);

  m_field->setText(query);
  m_field->setFocus();
  UpdateGroups();
  UpdateViews();
}

void SearchWindow::UpdateViews()
{
  bool const empty = m_field->text().isEmpty();
  bool const hasResults = m_model.rowCount() > 0;
  m_categories->setVisible(empty);
  m_groups->setVisible(!empty && m_groups->count() > 0);
  m_results->setVisible(!empty && hasResults);
  m_status->setVisible(!empty && !hasResults);
  m_status->setText(m_model.searching() ? tr("Searching…")
                                        : tr("No results.\nDownload the map of this area to search it offline."));
  m_showAll->setEnabled(hasResults);
}

void SearchWindow::UpdateGroups()
{
  m_groups->clear();
  for (auto const & group : m_model.groups())
  {
    auto const map = group.toMap();
    new QListWidgetItem(TwoLines(map.value(QStringLiteral("label")).toString(), map.value(QStringLiteral("hint")).toString()),
                        m_groups);
  }
  // As tall as its rows, so the results below get the rest.
  m_groups->setFixedHeight(m_groups->count() == 0 ? 0 : m_groups->sizeHintForRow(0) * m_groups->count() + 2 * m_groups->frameWidth());
  UpdateViews();
}

void SearchWindow::Activate(QModelIndex const & index)
{
  auto const suggestion = m_model.activate(index.row());
  if (!suggestion.isEmpty())
  {
    m_field->setText(suggestion);
    return;
  }
  m_manager.backToMap();
}
