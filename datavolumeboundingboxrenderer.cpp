#include "datavolumeboundingboxrenderer.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <iostream>

DataVolumeBoundingBoxRenderer::DataVolumeBoundingBoxRenderer()
    : vertexBuffer(QOpenGLBuffer::VertexBuffer) {
  initOpenGLShaders();
  initBoundingBoxGeometry();
}

DataVolumeBoundingBoxRenderer::~DataVolumeBoundingBoxRenderer() {
  vertexBuffer.destroy();
}

auto DataVolumeBoundingBoxRenderer::drawBoundingBox(QMatrix4x4 mvpMatrix)
    -> void {
  shaderProgram.link();
  // // Tell OpenGL to use the shader program of this class.
  shaderProgram.bind();

  // // Bind the vertex array object that links to the bounding box vertices.
  vertexArrayObject.bind();

  // // Set the model-view-projection matrix as a uniform value.
  shaderProgram.setUniformValue("mvpMatrix", mvpMatrix);

  // // Issue OpenGL draw commands.
  QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
  f->glLineWidth(2);
  f->glDrawArrays(GL_LINE_STRIP, 0, 16);

  // // Release objects until next render cycle.
  vertexArrayObject.release();
  shaderProgram.release();
}

auto DataVolumeBoundingBoxRenderer::initOpenGLShaders() -> void {
  if (!shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                             "../glsl/lines_vshader.glsl")) {
    std::cout << "Vertex shader error:\n"
              << shaderProgram.log().toStdString() << "\n"
              << std::flush;
    return;
  }

  if (!shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                             "../glsl/lines_fshader.glsl")) {
    std::cout << "Fragment shader error:\n"
              << shaderProgram.log().toStdString() << "\n"
              << std::flush;
    return;
  }

  if (!shaderProgram.link()) {
    std::cout << "Shader link error:\n"
              << shaderProgram.log().toStdString() << "\n"
              << std::flush;
    return;
  }
}

auto DataVolumeBoundingBoxRenderer::initBoundingBoxGeometry() -> void {
  // Vertices of a unit cube that represents the bounding box.
  const unsigned int numVertices = 16;
  float unitCubeVertices[numVertices][3] = {
      {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1},
      {1, 0, 1}, {1, 0, 0}, {1, 0, 1}, {1, 1, 1}, {1, 1, 0}, {1, 1, 1},
      {0, 1, 1}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}};

  // Create vertex buffer and upload vertex data to buffer.
  vertexBuffer.create();  // make sure to destroy in destructor!
  vertexBuffer.bind();
  vertexBuffer.allocate(unitCubeVertices, numVertices * 3 * sizeof(float));
  vertexBuffer.release();

  // Store the information OpenGL needs for rendering the vertex buffer
  // in a "vertex array object". This can easily be bound to the OpenGL
  // pipeline during rendering.
  QOpenGLVertexArrayObject::Binder vaoBinder(&vertexArrayObject);
  if (vertexArrayObject.isCreated()) {
    vertexBuffer.bind();
    shaderProgram.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3,
                                     3 * sizeof(float));
    shaderProgram.enableAttributeArray("vertexPosition");
    vertexBuffer.release();
  }
}
