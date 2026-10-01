#pragma once

#include "drape_frontend/user_event_stream.hpp"

#include "geometry/point2d.hpp"

#include <QPointer>
#include <QRectF>
#include <QQuickItem>
#include <QSize>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

#include <qopengl.h>

class QQuickWindow;

class MapView : public QQuickItem
{
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(bool primary READ primary WRITE setPrimary NOTIFY primaryChanged)
  Q_PROPERTY(bool interactive READ interactive WRITE setInteractive NOTIFY interactiveChanged)
  Q_PROPERTY(bool active READ active NOTIFY activeChanged)
  Q_PROPERTY(QRectF visibleArea READ visibleArea WRITE setVisibleArea NOTIFY visibleAreaChanged)

public:
  explicit MapView(QQuickItem * parent = nullptr);
  ~MapView() override;

  bool primary() const { return m_primary; }
  void setPrimary(bool primary);
  bool interactive() const { return m_interactive; }
  void setInteractive(bool interactive);
  bool active() const;
  QRectF visibleArea() const { return m_visibleArea; }
  void setVisibleArea(QRectF const & area);

  static void SetPinchSpeed(double speed);
  static void SetScrollSpeed(double speed);

  bool IsShowing() const;
  QSize PixelSize() const;
  double Ratio() const;

signals:
  void primaryChanged();
  void interactiveChanged();
  void activeChanged();
  void visibleAreaChanged();

protected:
  QSGNode * updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData *) override;
  void geometryChange(QRectF const & newGeometry, QRectF const & oldGeometry) override;
  void itemChange(ItemChange change, ItemChangeData const & value) override;
  bool eventFilter(QObject * watched, QEvent * event) override;
  void touchEvent(QTouchEvent * event) override;
  void touchUngrabEvent() override;
  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  void wheelEvent(QWheelEvent * event) override;

private:
  void WatchWindow(QQuickWindow * window);
  void ScheduleReevaluate();
  void UpdateInputFlags();
  void SendMouse(QMouseEvent * event, int type);
  void SendTouches(df::TouchEvent::ETouchType type, m2::PointD const * locations, int count);

  QPointer<QQuickWindow> m_window;
  QTimer m_frameTimer;
  QRectF m_visibleArea;
  bool m_primary = false;
  bool m_interactive = true;
  bool m_reevaluatePending = false;
  int m_pinchIds[2] = {-1, -1};
  double m_pinchStartDistance = 0.0;
  m2::PointD m_pinchCenter;
  int m_scrollId = -1;
  m2::PointD m_scrollOrigin;
  bool m_touchOpen = false;
  m2::PointD m_lastLocations[2];
  int m_lastCount = 0;
  GLuint m_textureId = 0;
  QSize m_textureSize;
};
