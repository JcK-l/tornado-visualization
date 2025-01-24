#include "opengldisplaywidget.h"

#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QPainter>
#include <cmath>
#include <iostream>

#include "HorizontalContourLinesRenderer.h"
#include "datavolumeboundingboxrenderer.h"

OpenGLDisplayWidget::OpenGLDisplayWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      distanceToCamera(-8.0),
      addOn(None),
      frameTime(new QElapsedTimer()),
      timer(new QBasicTimer()),
      frame(0),
      frameCounter(0),
      fps(0),
      isAnimated(false),
      framerateCap(60) {
  setFocusPolicy(Qt::StrongFocus);
}

OpenGLDisplayWidget::~OpenGLDisplayWidget() {
  // Clean up visualization pipeline.
  delete bboxRenderer;
  // ....
}

auto OpenGLDisplayWidget::openGLString() -> QString {
  QString profileStr;
  switch (format().profile()) {
    case QSurfaceFormat::NoProfile:
      profileStr = "no profile";
      break;
    case QSurfaceFormat::CompatibilityProfile:
      profileStr = "compatibility profile";
      break;
    case QSurfaceFormat::CoreProfile:
      profileStr = "core profile";
      break;
  }

  return QString("%1.%2 (%3)")
      .arg(format().majorVersion())
      .arg(format().minorVersion())
      .arg(profileStr);
}

auto OpenGLDisplayWidget::initializeGL() -> void {
  // Query and display some information about the used OpenGL context.
  std::cout << "Initializing OpenGLDisplayWidget with OpenGL version "
            << openGLString().toStdString() << ".\n"
            << std::flush;

  // Set the backgound color of the OpenGL display enable the depth buffer.
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
  f->glClearColor(0, 0, 0, 1);
  f->glEnable(GL_DEPTH_TEST);

  // Our own initialization of the visualization pipeline.
  initVisualizationPipeline();
  frameTime->start();
}

auto OpenGLDisplayWidget::resizeGL(int w, int h) -> void {
  // Calculate aspect ratio of the current viewport.
  float aspectRatio = float(w) / std::max(1, h);

  // Reset projection and set new perspective projection.
  projectionMatrix.setToIdentity();
  projectionMatrix.perspective(45.0, aspectRatio, 0.05, 25.0);

  // Update model-view-projection matrix with new projection.
  updateMVPMatrix();
}

auto OpenGLDisplayWidget::paintGL() -> void {
  // Clear color and depth buffer.
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
  f->glEnable(GL_POLYGON_OFFSET_FILL);
  f->glEnable(GL_LINE_SMOOTH);
  f->glPolygonOffset(2.0, 2.0);
  f->glEnable(GL_DEPTH_TEST);
  f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Call renderer modules.

  bboxRenderer->drawBoundingBox(mvpMatrix);

  hsliceRenderer->drawImage(mvpMatrix);

  switch (addOn) {
    case IsoAndStream:
      streamLinesRenderer->drawStreamLines(mvpMatrix);
      activeContourRenderer->drawContour(mvpMatrix);
      break;
    case IsoLines:
      activeContourRenderer->drawContour(mvpMatrix);
      break;
    case StreamLines:
      streamLinesRenderer->drawStreamLines(mvpMatrix);
      break;
    default:;
  }

  // ....
  displayUI();
}

auto OpenGLDisplayWidget::mousePressEvent(QMouseEvent *e) -> void {
  // Save the current position of the mouse pointer for subsequent use
  // in mouseMoveEvent().
  lastMousePosition = QVector2D(e->localPos());
}

auto OpenGLDisplayWidget::mouseMoveEvent(QMouseEvent *e) -> void {
  // If the user holds the left mouse button while moving the mouse, update
  // the rotation angles that specify from which side the grid visualization
  // is viewed.
  if (e->buttons() & Qt::LeftButton) {
    // Vector that points from the last stored position of the mouse
    // pointer to the current position.
    QVector2D mousePosDifference = QVector2D(e->localPos()) - lastMousePosition;

    // Update rotation angles in x and y direction. The factor "10" is an
    // arbitrary scaling constant that controls the sensitivity of the
    // mouse.
    rotationAngles.setX(
        fmod(rotationAngles.x() + mousePosDifference.x() / 10., 360.));
    rotationAngles.setY(
        fmod(rotationAngles.y() + mousePosDifference.y() / 10., 360.));

    // Store current position of mouse pointer for next call to this method.
    lastMousePosition = QVector2D(e->localPos());

    // Update model-view-projection matrix with new rotation angles.
    updateMVPMatrix();

    // Redraw OpenGL.
    if (!isAnimated) {
      update();
    }
  }
}

auto OpenGLDisplayWidget::wheelEvent(QWheelEvent *e) -> void {
  // Update distance of the camera to the rendered visualization. The factor
  // "500" is arbitrary and controls that sensitivity of the mouse.
  //distanceToCamera += e->delta() / 500.;
  distanceToCamera += e->angleDelta().y() / 500.0;

  // Update model-view-projection matrix with new distance to camera.
  updateMVPMatrix();

  // Redraw OpenGL.
  if (!isAnimated) {
    update();
  }
}

auto OpenGLDisplayWidget::timerEvent(QTimerEvent *event) -> void {
  frame += 1;
  frameCounter += 1;

  if (frameTime->elapsed() >= 1000) {
    fps = frameCounter;
    frameCounter = 0;
    frameTime->restart();
  }

  flowDataSource->setFrame(frame);
  hsliceRenderer->updateTexture();

  switch (addOn) {
    case IsoAndStream:
      activeContourRenderer->updateIso();
      streamLinesRenderer->updateStreamLines();
      break;
    case IsoLines:
      activeContourRenderer->updateIso();
      break;
    case StreamLines:
      streamLinesRenderer->updateStreamLines();
      break;
    default:;
  }

  update();
}

auto OpenGLDisplayWidget::keyPressEvent(QKeyEvent *e) -> void {
  currentKeybindings(e);
  // Redraw OpenGL.
  if (!isAnimated) {
    update();
  }
}

auto OpenGLDisplayWidget::updateMVPMatrix() -> void {
  // Calculate a simple model view transformation from rotation angles
  // and distance to camera.
  // NOTE: Read from bottom to top.

  QMatrix4x4 mvMatrix;
  mvMatrix.translate(0.0, 0.0, distanceToCamera);
  mvMatrix.rotate(rotationAngles.y(), QVector3D(1.0, 0.0, 0.0));
  mvMatrix.rotate(rotationAngles.x(), QVector3D(0.0, 1.0, 0.0));
  mvMatrix.rotate(-90, QVector3D(1.0, 0.0, 0.0));
  mvMatrix.translate(-1.0, -1.0, -1.0);
  mvMatrix.scale(2.0);

  mvpMatrix = projectionMatrix * mvMatrix;
}

auto OpenGLDisplayWidget::initVisualizationPipeline() -> void {
  // Initialize the visualization pipeline:

  // Initialize data source(s).
  // ....
  flowDataSource = new FlowDataSource(32);

  // Initialize mapper modules.
  // ....
  hsliceMapper = new HorizontalSliceToImageMapper(flowDataSource);
  hcontourMapper = new HorizontalSliceToContourLineMapper(flowDataSource);
  streamLinesMapper = new StreamLinesMapper(flowDataSource);
  glslContourRenderer = new ContourRendererGLSL(flowDataSource);

  // Initialize rendering modules.
  bboxRenderer = new DataVolumeBoundingBoxRenderer();
  hsliceRenderer = new HorizontalSliceRenderer(hsliceMapper);
  hcontourRenderer = new HorizontalContourLinesRenderer(hcontourMapper);
  activeContourRenderer = hcontourRenderer;
  streamLinesRenderer = new StreamLinesRenderer(streamLinesMapper);
  // ....
}
auto OpenGLDisplayWidget::setAddOn(AddOn inAddOn) -> void {
  addOn = (inAddOn == None) ? None : static_cast<AddOn>(addOn ^ inAddOn);
}

auto OpenGLDisplayWidget::displayUI() -> void {
  QPainter painter(this);
  painter.setPen(Qt::white);
  painter.setFont(QFont("Noto Sans", 16));

  // keybindings
  painter.drawText(width() - 200, 5, width(), height(), Qt::AlignTop,
                   QString("Keybindings"));
  QString title;
  int shiftFaktor = 3;

  switch (hsliceRenderer->getMode()) {
    case Data:
      dataUI(painter, 65, 165);
      if (activeContourRenderer->getIsoState()) {
        title = QString("IsoEdit");
        activeIsoUI(painter, 5, 45);
      } else if (streamLinesRenderer->getStreamState()) {
        title = QString("StreamEdit");
        activeStreamUI(painter, 5, 45);
      } else {
        title = QString("Data");
        defaultUI(painter, 5, 45);
      }
      break;
    case Default:
      painter.drawText(width() / 2 - 7 * shiftFaktor, 5, width(), height(),
                       Qt::AlignTop, QString("Picture"));
      defaultUI(painter, 5, 45);
      break;
  }
  switch (addOn) {
    case IsoAndStream:
      title = QString("%1  |  IsoAndStream").arg(title);
      painter.drawText(width() / 2 - title.size() * shiftFaktor, 5, width(),
                       height(), Qt::AlignTop, title);
      streamLineUI(painter, 185, 425);
      isoUI(painter, 325, 605);
      break;
    case IsoLines:
      title = QString("%1  |  IsoLines").arg(title);
      painter.drawText(width() / 2 - title.size() * shiftFaktor, 5, width(),
                       height(), Qt::AlignTop, title);
      isoUI(painter, 185, 425);
      break;
    case StreamLines:;
      title = QString("%1  |  StreamLines").arg(title);
      painter.drawText(width() / 2 - title.size() * shiftFaktor, 5, width(),
                       height(), Qt::AlignTop, title);
      streamLineUI(painter, 185, 425);
      break;
    default:
      if (hsliceRenderer->getMode() == Data) {
        title = QString("%1  |  None").arg(title);
        painter.drawText(width() / 2 - title.size() * shiftFaktor, 5, width(),
                         height(), Qt::AlignTop, title);
      }
  }
  painter.end();
}

auto OpenGLDisplayWidget::currentKeybindings(QKeyEvent *e) -> void {
  switch (hsliceRenderer->getMode()) {
    case Data:
      dataKeybinding(e);
    case Default:
      if (activeContourRenderer->getIsoState()) {
        isoActiveKeybinding(e);
      } else if (streamLinesRenderer->getStreamState()) {
        streamActiveKeybinding(e);
      } else {
        defaultKeybinding(e);
      }
      break;
  }

  switch (addOn) {
    case IsoAndStream:
      isoKeybinding(e);
      streamLineKeybinding(e);
      break;
    case IsoLines:
      isoKeybinding(e);
      break;
    case StreamLines:
      streamLineKeybinding(e);
      break;
    default:;
  }
}

auto OpenGLDisplayWidget::defaultKeybinding(QKeyEvent *e) -> void {
  switch (e->key()) {
    case Qt::Key_Left:
      flowDataSource->setDimension(-8);
      hsliceRenderer->setMaxSteps();
      glslContourRenderer->setMaxSteps();
      hcontourRenderer->setMaxSteps();
      streamLinesRenderer->setMaxSteps();
      break;
    case Qt::Key_Right:
      flowDataSource->setDimension(8);
      hsliceRenderer->setMaxSteps();
      glslContourRenderer->setMaxSteps();
      hcontourRenderer->setMaxSteps();
      streamLinesRenderer->setMaxSteps();
      break;
    case Qt::Key_Down:
      hsliceRenderer->moveSlice(-1);
      glslContourRenderer->moveSlice(-1);
      hcontourRenderer->moveSlice(-1);
      break;
    case Qt::Key_Up:
      hsliceRenderer->moveSlice(1);
      glslContourRenderer->moveSlice(1);
      hcontourRenderer->moveSlice(1);
      break;
    case Qt::Key_M:
      setAddOn(None);
      hsliceRenderer->setMode(Data);
      break;
  }
}

auto OpenGLDisplayWidget::dataKeybinding(QKeyEvent *e) -> void {
  switch (e->key()) {
    case Qt::Key_X:
      hsliceRenderer->setWindComponent(0);
      glslContourRenderer->setWindComponent(0);
      hcontourRenderer->setWindComponent(0);
      break;
    case Qt::Key_Y:
      hsliceRenderer->setWindComponent(1);
      glslContourRenderer->setWindComponent(1);
      hcontourRenderer->setWindComponent(1);
      break;
    case Qt::Key_Z:
      hsliceRenderer->setWindComponent(2);
      glslContourRenderer->setWindComponent(2);
      hcontourRenderer->setWindComponent(2);
      break;
    case Qt::Key_B:
      hsliceRenderer->setWindComponent(3);
      glslContourRenderer->setWindComponent(3);
      hcontourRenderer->setWindComponent(3);
      break;
    case Qt::Key_I:
      activeContourRenderer->toggleIsoEdit(false);
      setAddOn(IsoLines);
      break;
    case Qt::Key_S:
      streamLinesRenderer->toggleStreamEdit(false);
      setAddOn(StreamLines);
      break;
    case Qt::Key_H:
      hsliceRenderer->toggleHCL(true);
      break;
    case Qt::Key_G:
      //hsliceRenderer->moveSlice(-flowDataSource->getDimension());
      isGLSL = !isGLSL;
      activeContourRenderer = isGLSL ? glslContourRenderer : hcontourRenderer;
      break;
    case Qt::Key_1:
      if (!timer->isActive()) {
        isAnimated = true;
        timer->start(1000 / framerateCap, this);
      }
      break;
    case Qt::Key_2:
      if (timer->isActive()) {
        isAnimated = false;
        fps = 0;
        timer->stop();
      }
      break;
    case Qt::Key_3:
      framerateCap = (framerateCap - 15 < 15) ? 15 : framerateCap - 15;
      if (timer->isActive()) {
        isAnimated = true;
        timer->stop();
        timer->start(1000 / framerateCap, this);
      }
      break;
    case Qt::Key_4:
      framerateCap += 15;
      if (timer->isActive()) {
        isAnimated = true;
        timer->stop();
        timer->start(1000 / framerateCap, this);
      }
      break;
  }
}

auto OpenGLDisplayWidget::isoKeybinding(QKeyEvent *e) -> void {
  switch (e->key()) {
    case Qt::Key_E:
      streamLinesRenderer->toggleStreamEdit(false);
      activeContourRenderer->toggleIsoEdit(true);
      break;
    case Qt::Key_Plus:
      activeContourRenderer->addIsoLine();
      break;
    case Qt::Key_Minus:
      activeContourRenderer->deleteIsoLine();
      break;
  }
}

auto OpenGLDisplayWidget::isoActiveKeybinding(QKeyEvent *e) -> void {
  switch (e->key()) {
    case Qt::Key_Left:
      activeContourRenderer->moveActiveIso(-0.02);
      break;
    case Qt::Key_Right:
      activeContourRenderer->moveActiveIso(0.02);
      break;
    case Qt::Key_Down:
      activeContourRenderer->setActiveIso(-1);
      break;
    case Qt::Key_Up:
      activeContourRenderer->setActiveIso(1);
      break;
    case Qt::Key_M:
      setAddOn(None);
      activeContourRenderer->toggleIsoEdit(false);
      hsliceRenderer->setMode(Data);
      break;
  }
}

auto OpenGLDisplayWidget::streamActiveKeybinding(QKeyEvent *e) -> void {
  switch (e->key()) {
    case Qt::Key_Left:
      streamLinesRenderer->setCurrentIntegration(-1);
      break;
    case Qt::Key_Right:
      streamLinesRenderer->setCurrentIntegration(1);
      break;
    case Qt::Key_Down:
      streamLinesRenderer->setTValue(-0.1);
      break;
    case Qt::Key_Up:
      streamLinesRenderer->setTValue(0.1);
      break;
    case Qt::Key_M:
      setAddOn(None);
      streamLinesRenderer->toggleStreamEdit(false);
      hsliceRenderer->setMode(Data);
      break;
  }
}

auto OpenGLDisplayWidget::streamLineKeybinding(QKeyEvent *e) -> void {
  switch (e->key()) {
    case Qt::Key_U:
      activeContourRenderer->toggleIsoEdit(false);
      streamLinesRenderer->toggleStreamEdit(true);
      break;
    case Qt::Key_P:
      streamLinesRenderer->togglePathLines(true);
      break;
    case Qt::Key_O:
      streamLinesRenderer->toggleShiftingSeeds(true);
      break;
    case Qt::Key_R:
      streamLinesRenderer->restartPathLines();
      break;
    case Qt::Key_5:
      streamLinesRenderer->setShiftingSeedsInterval(-2);
      break;
    case Qt::Key_6:
      streamLinesRenderer->setShiftingSeedsInterval(2);
      break;
    case Qt::Key_7:
      streamLinesRenderer->setPathLinesInterval(-1);
      break;
    case Qt::Key_8:
      streamLinesRenderer->setPathLinesInterval(1);
      break;
  }
}

const int marginRight = 250;

auto OpenGLDisplayWidget::defaultUI(QPainter &painter, int left, int right)
    -> void {
  // Indicators
  painter.drawText(5, left, width(), height(), Qt::AlignTop,
                   QString("Step: %1").arg(hsliceRenderer->getSteps()));

  painter.drawText(5, left + 20, width(), height(), Qt::AlignTop,
                   QString("maxSteps: %1").arg(flowDataSource->getDimension()));
  // default arrow keys
  painter.drawText(width() - marginRight, right, width(), height(), Qt::AlignTop,
                   QString("Increase Step: up"));
  painter.drawText(width() - marginRight, right + 20, width(), height(),
                   Qt::AlignTop,
                   QString("Decrease Step: down"));
  painter.drawText(width() - marginRight, right + 40, width(), height(),
                   Qt::AlignTop,
                   QString("Increase maxSteps: right"));
  painter.drawText(width() - marginRight, right + 60, width(), height(),
                   Qt::AlignTop,
                   QString("Decrease maxSteps: left"));

  painter.drawText(width() - marginRight, right + 80, width(), height(),
                   Qt::AlignTop,
                   QString("Toggle Mode: m"));
}

auto OpenGLDisplayWidget::dataUI(QPainter &painter, int left, int right)
    -> void {
  painter.drawText(
      5, left, width(), height(), Qt::AlignTop,
      QString("Component: %1").arg(hsliceRenderer->getWindComponent()));
  painter.drawText(5, left + 20, width(), height(), Qt::AlignTop,
                   QString("Fps: %1").arg(fps));
  painter.drawText(5, left + 40, width(), height(), Qt::AlignTop,
                   QString("FramerateCap: %1").arg(framerateCap));
  painter.drawText(5, left + 60, width(), height(), Qt::AlignTop,
                   QString("GLSL: %1").arg(isGLSL));
  painter.drawText(5, left + 80, width(), height(), Qt::AlignTop,
                   QString("HCL: %1").arg(hsliceRenderer->isHCL()));

  painter.drawText(width() - marginRight, right, width(), height(),
                   Qt::AlignTop,
                   QString("Toggle GLSL: g"));
  painter.drawText(width() - marginRight, right + 20, width(), height(),
                   Qt::AlignTop,
                   QString("Toggle HCL: h"));
  painter.drawText(width() - marginRight, right + 40, width(), height(),
                   Qt::AlignTop,
                   QString("Toggle IsoLines: i"));
  painter.drawText(width() - marginRight, right + 60, width(), height(),
                   Qt::AlignTop,
                   QString("Toggle StreamLines: s"));
  painter.drawText(width() - marginRight, right + 80, width(), height(),
                   Qt::AlignTop,
                   QString("Wind Component: x"));
  painter.drawText(width() - marginRight, right + 100, width(), height(),
                   Qt::AlignTop,
                   QString("Wind Component: y"));
  painter.drawText(width() - marginRight, right + 120, width(), height(),
                   Qt::AlignTop,
                   QString("Wind Component: z"));
  painter.drawText(width() - marginRight, right + 140, width(), height(),
                   Qt::AlignTop,
                   QString("Wind Component: b"));
  painter.drawText(width() - marginRight, right + 160, width(), height(),
                   Qt::AlignTop,
                   QString("Start Animation: 1"));
  painter.drawText(width() - marginRight, right + 180, width(), height(),
                   Qt::AlignTop,
                   QString("Stop Animation: 2"));
  painter.drawText(width() - marginRight, right + 200, width(), height(),
                   Qt::AlignTop,
                   QString("Decrease Animation: 3"));
  painter.drawText(width() - marginRight, right + 220, width(), height(),
                   Qt::AlignTop,
                   QString("Increase Animation: 4"));
}

auto OpenGLDisplayWidget::isoUI(QPainter &painter, int left, int right)
    -> void {
  for (int i = 0; i < activeContourRenderer->getValuesArray().size(); i++) {
    painter.setPen(Qt::white);
    if (i == activeContourRenderer->getCurrentActiveValue() &&
        activeContourRenderer->getIsoState())
      painter.setPen(Qt::red);
    painter.drawText(5, left + i * 20, width(), height(), Qt::AlignTop,
                     QString("C%1: %2").arg(i).arg(
                         activeContourRenderer->getValuesArray().at(i)));
  }
  painter.setPen(Qt::white);

  painter.drawText(width() - marginRight, right, width(), height(),
                   Qt::AlignTop,
                   QString("Add IsoLine: +"));
  painter.drawText(width() - marginRight, right + 20, width(), height(),
                   Qt::AlignTop,
                   QString("Sub IsoLine: -"));
  painter.drawText(width() - marginRight, right + 40, width(), height(),
                   Qt::AlignTop,
                   QString("Edit IsoLines : e"));
}

auto OpenGLDisplayWidget::activeIsoUI(QPainter &painter, int left, int right)
    -> void {
  painter.drawText(5, left, width(), height(), Qt::AlignTop,
                   QString("Step: %1").arg(hsliceRenderer->getSteps()));

  painter.drawText(5, left + 20, width(), height(), Qt::AlignTop,
                   QString("maxSteps: %1").arg(flowDataSource->getDimension()));
  // edit Iso arrow keys
  painter.drawText(width() - marginRight, right, width(), height(),
                   Qt::AlignTop,
                   QString("Select Active Iso: up"));
  painter.drawText(width() - marginRight, right + 20, width(), height(),
                   Qt::AlignTop,
                   QString("Select Active Iso: down"));
  painter.drawText(width() - marginRight, right + 40, width(), height(),
                   Qt::AlignTop,
                   QString("Increase Iso Value: right"));
  painter.drawText(width() - marginRight, right + 60, width(), height(),
                   Qt::AlignTop,
                   QString("Decrease Iso Value: left"));
  painter.drawText(width() - marginRight, right + 80, width(), height(),
                   Qt::AlignTop,
                   QString("Toggle Mode: m"));
}

auto OpenGLDisplayWidget::activeStreamUI(QPainter &painter, int left, int right)
    -> void {
  // Indicators
  painter.drawText(5, left, width(), height(), Qt::AlignTop,
                   QString("Step: %1").arg(hsliceRenderer->getSteps()));

  painter.drawText(5, left + 20, width(), height(), Qt::AlignTop,
                   QString("maxSteps: %1").arg(flowDataSource->getDimension()));
  // default arrow keys
  painter.drawText(width() - marginRight, right, width(), height(),
                   Qt::AlignTop,
                   QString("Increase T Value: up"));
  painter.drawText(width() - marginRight, right + 20, width(), height(),
                   Qt::AlignTop,
                   QString("Decrease T Value: down"));
  painter.drawText(width() - marginRight, right + 40, width(), height(),
                   Qt::AlignTop,
                   QString("Select Integration: right"));
  painter.drawText(width() - marginRight, right + 60, width(), height(),
                   Qt::AlignTop,
                   QString("Select Integration: left"));

  painter.drawText(width() - marginRight, right + 80, width(), height(),
                   Qt::AlignTop,
                   QString("Toggle Mode: m"));
}

auto OpenGLDisplayWidget::streamLineUI(QPainter &painter, int left, int right)
    -> void {
  QString result;
  switch (streamLinesRenderer->getIntegration()) {
    case Euler:
      result = QString("Euler");
      break;
    case Kutta2:
      result = QString("Kutta2");
      break;
    case Kutta4:
      result = QString("Kutta4");
      break;
  }
  painter.drawText(5, left, width(), height(), Qt::AlignTop,
                   QString("Integration: %1").arg(result));

  painter.drawText(
      5, left + 20, width(), height(), Qt::AlignTop,
      QString("T Value: %1").arg(streamLinesRenderer->getTValue()));

  painter.drawText(
      5, left + 40, width(), height(), Qt::AlignTop,
      QString("PathLines: %1").arg(streamLinesRenderer->getPathState()));
  painter.drawText(
      5, left + 60, width(), height(), Qt::AlignTop,
      QString("Shifting: %1").arg(streamLinesRenderer->getShiftingState()));
  painter.drawText(5, left + 80, width(), height(), Qt::AlignTop,
                   QString("Shift Interval: %1")
                       .arg(streamLinesRenderer->getShiftingSeedsInterval()));
  painter.drawText(5, left + 100, width(), height(), Qt::AlignTop,
                   QString("PathLines Interval: %1")
                       .arg(streamLinesRenderer->getPathLinesInterval()));

  painter.drawText(width() - marginRight, right, width(), height(),
                   Qt::AlignTop,
                   QString("Edit Streamline: u"));

  painter.drawText(width() - marginRight, right + 20, width(), height(),
                   Qt::AlignTop,
                   QString("Toggle PathLines: p"));

  painter.drawText(width() - marginRight, right + 40, width(), height(),
                   Qt::AlignTop,
                   QString("Restart Lines: r"));

  painter.drawText(width() - marginRight, right + 60, width(), height(),
                   Qt::AlignTop,
                   QString("Toggle Shifting: o"));

  painter.drawText(width() - marginRight, right + 80, width(), height(),
                   Qt::AlignTop,
                   QString("Decrease Shift Interval: 5"));

  painter.drawText(width() - marginRight, right + 100, width(), height(),
                   Qt::AlignTop,
                   QString("Increase Shift Interval: 6"));

  painter.drawText(width() - marginRight, right + 120, width(), height(),
                   Qt::AlignTop,
                   QString("Decrease Path Interval: 7"));

  painter.drawText(width() - marginRight, right + 140, width(), height(),
                   Qt::AlignTop,
                   QString("Increase Path Interval: 8"));
}
