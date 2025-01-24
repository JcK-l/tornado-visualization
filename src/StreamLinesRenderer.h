//
// Created by Joshua Lowe on 26.06.22.
//

#ifndef CODE5_STREAMLINESRENDERER_H
#define CODE5_STREAMLINESRENDERER_H

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include "StreamLinesMapper.h"

class StreamLinesRenderer {
 public:
  StreamLinesRenderer();
  explicit StreamLinesRenderer(StreamLinesMapper*);
  virtual ~StreamLinesRenderer();

  // Draw the bounding box to the current OpenGL viewport.
  auto drawStreamLines(QMatrix4x4 mvpMatrix) -> void;
  auto setMaxSteps() -> void;
  auto setMapper(StreamLinesMapper*) -> void;
  auto updateStreamLines() -> void;
  auto setCurrentIntegration(int) -> void;
  auto toggleStreamEdit(bool state) -> void;
  auto getStreamState() -> bool;
  auto getTValue() -> float;
  auto getPathLinesInterval() -> int;
  auto getShiftingSeedsInterval() -> int;
  auto getShiftingState() -> bool;
  auto getPathState() -> bool;
  auto setTValue(float) -> void;
  auto getIntegration() -> Integration;
  auto toggleShiftingSeeds(bool) -> void;
  auto setShiftingSeedsInterval(int) -> void;
  auto togglePathLines(bool) -> void;
  auto setPathLinesInterval(int) -> void;
  auto restartPathLines() -> void;

 protected:
  auto createSeeds() -> void;
  auto initOpenGLShaders() -> void;
  auto initStreamLines() -> void;

  int maxSteps;
  int interval;
  bool isActive;
  bool isShift;

  QOpenGLShaderProgram shaderProgram;
  QOpenGLBuffer vertexBuffer;
  QOpenGLVertexArrayObject vertexArrayObject;

 private:
  std::vector<QVector3D> seeds;
  std::vector<QVector3D> streamLines;
  StreamLinesMapper* streamLinesMapper;
};

#endif  // CODE5_STREAMLINESRENDERER_H
