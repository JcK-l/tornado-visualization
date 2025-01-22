#ifndef OPENGLDISPLAYWIDGET_H
#define OPENGLDISPLAYWIDGET_H

#include <QBasicTimer>
#include <QElapsedTimer>
#include <QOpenGLWidget>

#include "ContourRendererGLSL.h"
#include "HorizontalContourLinesRenderer.h"
#include "HorizontalSliceRenderer.h"
#include "HorizontalSliceToContourLineMapper.h"
#include "HorizontalSliceToImageMapper.h"
#include "StreamLinesMapper.h"
#include "StreamLinesRenderer.h"
#include "datavolumeboundingboxrenderer.h"

enum AddOn { None, IsoLines, StreamLines, IsoAndStream };

class OpenGLDisplayWidget : public QOpenGLWidget {
  Q_OBJECT

 public:
  explicit OpenGLDisplayWidget(QWidget *parent = nullptr);
  ~OpenGLDisplayWidget() override;

  auto openGLString() -> QString;

 protected:
  auto initializeGL() -> void override;
  auto resizeGL(int w, int h) -> void override;
  auto paintGL() -> void override;
  auto timerEvent(QTimerEvent *) -> void override;

  auto mousePressEvent(QMouseEvent *e) -> void override;
  auto mouseMoveEvent(QMouseEvent *e) -> void override;
  auto wheelEvent(QWheelEvent *e) -> void override;

  auto keyPressEvent(QKeyEvent *e) -> void override;

 private:
  AddOn addOn;
  bool isAnimated;
  int frame;
  int frameCounter;
  float framerateCap;
  double fps;
  QBasicTimer *timer;
  QElapsedTimer *frameTime;

  auto setAddOn(AddOn) -> void;

  auto defaultUI(QPainter &, int, int) -> void;
  auto dataUI(QPainter &, int, int) -> void;
  auto isoUI(QPainter &, int, int) -> void;
  auto activeIsoUI(QPainter &, int, int) -> void;
  auto activeStreamUI(QPainter &, int, int) -> void;
  auto streamLineUI(QPainter &, int, int) -> void;

  auto displayUI() -> void;

  auto defaultKeybinding(QKeyEvent *) -> void;
  auto dataKeybinding(QKeyEvent *) -> void;
  auto isoKeybinding(QKeyEvent *) -> void;
  auto isoActiveKeybinding(QKeyEvent *) -> void;
  auto streamActiveKeybinding(QKeyEvent *) -> void;
  auto streamLineKeybinding(QKeyEvent *) -> void;

  auto currentKeybindings(QKeyEvent *) -> void;

  // VIEW PROJECTION:
  // ================

  // Matrices and related variables that control vertex transformation
  // from world space to OpenGL view space.
  QMatrix4x4 projectionMatrix;
  QMatrix4x4 mvpMatrix;
  QVector2D lastMousePosition;
  QVector2D rotationAngles;
  float distanceToCamera;

  // Recompute the mode-view-projection matrix from current rotation angles,
  // distance to camera, viewport geometry.
  auto updateMVPMatrix() -> void;

  // VIISUALIZATION PIPELINE:
  // ========================

  FlowDataSource *flowDataSource;
  StreamLinesRenderer *streamLinesRenderer;
  HorizontalSliceRenderer *hsliceRenderer;
  HorizontalContourLinesRenderer *hcontourRenderer;
  StreamLinesMapper *streamLinesMapper;
  HorizontalSliceToContourLineMapper *hcontourMapper;
  HorizontalSliceToImageMapper *hsliceMapper;
  DataVolumeBoundingBoxRenderer *bboxRenderer;
  // ....

  // Initialize the pipeline (create instances of data source, mapping,
  // rendering etc. modules and connect them).
  auto initVisualizationPipeline() -> void;
};

#endif  // OPENGLDISPLAYWIDGET_H
