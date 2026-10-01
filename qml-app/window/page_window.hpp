#pragma once

#include <QIdentityProxyModel>
#include <QMainWindow>

#include <functional>

// A window for one page (bookmarks, map downloads, search, ...), stacked on the window that opens it
// like conversations' SearchWindow: Hildon shows it on top with a back button.
class PageWindow : public QMainWindow
{
  Q_OBJECT

public:
  PageWindow(QString const & title, QWidget * parent);
};

// Shows a source model row as text computed from the row (two lines: name and details).
class DisplayProxy : public QIdentityProxyModel
{
public:
  using Text = std::function<QString(QModelIndex const & source)>;

  DisplayProxy(Text text, QObject * parent) : QIdentityProxyModel(parent), m_text(std::move(text)) {}

  QVariant data(QModelIndex const & index, int role) const override
  {
    if (role == Qt::DisplayRole)
      return m_text(mapToSource(index));
    return QIdentityProxyModel::data(index, role);
  }

private:
  Text m_text;
};

// Two lines, the second only when there is something to say.
inline QString TwoLines(QString const & first, QString const & second)
{
  return second.isEmpty() ? first : first + QLatin1Char('\n') + second;
}
