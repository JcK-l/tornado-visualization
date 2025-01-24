//
// Created by Joshua Lowe on 19.06.22.
//

#ifndef CODE4_HORIZONTALCONTOURLINESRENDERER_H
#define CODE4_HORIZONTALCONTOURLINESRENDERER_H

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

#include "HorizontalSliceToContourLineMapper.h"

#define MAX_ISO_AMOUNT 5

enum MoveIso { Inactive, Active };

class HorizontalContourLinesRenderer {
 public:
  HorizontalContourLinesRenderer();
  explicit HorizontalContourLinesRenderer(HorizontalSliceToContourLineMapper*);
  virtual ~HorizontalContourLinesRenderer();

  // Draw the bounding box to the current OpenGL viewport.
  virtual auto drawContour(QMatrix4x4 mvpMatrix) -> void;
  auto moveSlice(int) -> void;
  auto toggleIsoEdit(bool) -> void;
  auto getIsoState() -> bool;
  virtual auto setMaxSteps() -> void;
  virtual auto setWindComponent(int ic) -> void;
  virtual auto isGLSL() -> bool;
  auto setCurrentStep(int) -> void;
  auto setMapper(HorizontalSliceToContourLineMapper*) -> void;
  auto addIsoLine() -> void;
  auto deleteIsoLine() -> void;
  auto setActiveIso(int) -> void;
  auto moveActiveIso(float) -> void;
  auto getValuesArray() -> QVector<float>;
  auto getCurrentActiveValue() -> int;
  virtual auto updateIso() -> void;

 protected:
  virtual auto initOpenGLShaders() -> void;
  virtual auto initContourLines() -> void;

  bool isActive;
  int maxSteps;
  int currentStep;
  int currentActiveValue;
  QVector<GLfloat> isoValues;

  QOpenGLShaderProgram shaderProgram;
  QOpenGLBuffer vertexBuffer;
  QOpenGLVertexArrayObject vertexArrayObject;

  HorizontalSliceToContourLineMapper* contourMapper;

 private:
  QVector<QVector3D> isoLines;
  QVector<GLint> isoLinesSizes;
  int fragmentColor;
};

#endif  // CODE4_HORIZONTALCONTOURLINESRENDERER_H
