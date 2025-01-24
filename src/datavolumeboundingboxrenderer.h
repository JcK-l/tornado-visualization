#ifndef DATAVOLUMEBOUNDINGBOXRENDERER_H
#define DATAVOLUMEBOUNDINGBOXRENDERER_H

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

class DataVolumeBoundingBoxRenderer {
 public:
  DataVolumeBoundingBoxRenderer();
  virtual ~DataVolumeBoundingBoxRenderer();

  // Draw the bounding box to the current OpenGL viewport.
  auto drawBoundingBox(QMatrix4x4 mvpMatrix) -> void;

 private:
  auto initOpenGLShaders() -> void;
  auto initBoundingBoxGeometry() -> void;

  QOpenGLShaderProgram shaderProgram;
  QOpenGLBuffer vertexBuffer;
  QOpenGLVertexArrayObject vertexArrayObject;
};

#endif  // DATAVOLUMEBOUNDINGBOXRENDERER_H
