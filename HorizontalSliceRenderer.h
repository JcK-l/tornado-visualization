//
// Created by Joshua Lowe on 15.05.22.
//

#ifndef UNTITLED_HORIZONTALSLICERENDERER_H
#define UNTITLED_HORIZONTALSLICERENDERER_H

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>

#include "HorizontalSliceToImageMapper.h"

class HorizontalSliceRenderer {
 public:
  HorizontalSliceRenderer();
  explicit HorizontalSliceRenderer(HorizontalSliceToImageMapper*);
  virtual ~HorizontalSliceRenderer();

  // Draw the bounding box to the current OpenGL viewport.
  auto getMode() -> Mode;
  auto getSteps() -> int;
  auto getWindComponent() -> QString;
  auto drawImage(QMatrix4x4) -> void;
  auto setMapper(HorizontalSliceToImageMapper*) -> void;
  auto moveSlice(int) -> void;
  auto setMaxSteps() -> void;
  auto setWindComponent(int ic) -> void;
  auto setMode(Mode) -> void;
  auto toggleHCL(bool) -> void;
  auto updateTexture() -> void;
  auto isHCL() -> bool;

 private:
  auto initOpenGLShaders() -> void;
  auto initHorizontalSlice() -> void;
  auto newTexture() -> void;

  float sliceCorners[6][3];
  int maxSteps;
  int currentStep;

  QImage img;
  QOpenGLTexture* texture;
  HorizontalSliceToImageMapper* imageMapper;
  QOpenGLShaderProgram shaderProgram;
  QOpenGLBuffer vertexBuffer;
  QOpenGLVertexArrayObject vertexArrayObject;
};

#endif  // UNTITLED_HORIZONTALSLICERENDERER_H
