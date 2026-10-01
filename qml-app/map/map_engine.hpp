#pragma once

#include "qt/qt_common/qtoglcontextfactory.hpp"

#include "drape_frontend/gui/skin.hpp"

#include "geometry/rect2d.hpp"

#include <QObject>
#include <QPointer>
#include <QSize>

#include <memory>
#include <optional>
#include <vector>

class Framework;
class MapView;

class MapEngine : public QObject
{
  Q_OBJECT

public:
  explicit MapEngine(Framework & framework);
  ~MapEngine() override;

  static MapEngine & Instance();

  Framework & GetFramework() { return m_framework; }
  qt::common::QtOGLContextFactory * GetContextFactory() { return m_contextFactory.get(); }

  void Attach(MapView * view);
  void Detach(MapView * view);
  void Reevaluate();
  void ApplyVisibleArea(MapView const * view);

  bool IsActive(MapView const * view) const { return m_active == view; }
  bool IsCreated() const { return m_created; }

signals:
  void activeViewChanged();

private:
  void Create(QSize pixelSize, double ratio);
  void Resize(QSize pixelSize);
  void Activate(MapView * view);

  Framework & m_framework;
  std::unique_ptr<qt::common::QtOGLContextFactory> m_contextFactory;
  std::unique_ptr<gui::Skin> m_skin;
  std::vector<QPointer<MapView>> m_views;
  QPointer<MapView> m_active;
  std::optional<m2::RectD> m_primaryViewport;
  QSize m_size;
  bool m_created = false;
  bool m_foreground = false;
};

inline Framework & AppFramework()
{
  return MapEngine::Instance().GetFramework();
}
