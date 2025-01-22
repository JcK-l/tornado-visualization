//
// Created by Joshua Lowe on 26.06.22.
//

#include "StreamLinesRenderer.h"

#include <QOpenGLFunctions>
#include <iostream>
StreamLinesRenderer::StreamLinesRenderer()
    : vertexBuffer(QOpenGLBuffer::VertexBuffer),
      maxSteps(31),
      isActive(false) {}

StreamLinesRenderer::StreamLinesRenderer(StreamLinesMapper *mapper)
    : vertexBuffer(QOpenGLBuffer::VertexBuffer),
      streamLinesMapper(mapper),
      maxSteps(mapper->getDimension() - 1),
      isActive(false),
      isShift(false),
      interval(30) {
  createSeeds();
  initOpenGLShaders();
  vertexBuffer.create();
  updateStreamLines();
}

StreamLinesRenderer::~StreamLinesRenderer() { vertexBuffer.destroy(); }

auto StreamLinesRenderer::togglePathLines(bool state) -> void {
  streamLinesMapper->togglePathLines(state);
  createSeeds();
  updateStreamLines();
}

auto StreamLinesRenderer::toggleShiftingSeeds(bool state) -> void {
  isShift = state && !isShift;
}

auto StreamLinesRenderer::setShiftingSeedsInterval(int step) -> void {
  interval = (interval + step < 1) ? 1 : interval + step;
}

auto StreamLinesRenderer::setPathLinesInterval(int step) -> void {
  streamLinesMapper->setPathLinesInterval(step);
}

auto StreamLinesRenderer::createSeeds() -> void {
  seeds.clear();
  float value;
  for (int i = 0; i < 2 * maxSteps; i++) {
    value = i / (float)2;
    seeds.push_back(
        QVector3D((float)maxSteps / 2.0, (float)maxSteps / 2.0, value));
  }
}

auto StreamLinesRenderer::restartPathLines() -> void {
  streamLinesMapper->clearPathLinesSeeds();
  createSeeds();
  updateStreamLines();
}

auto StreamLinesRenderer::drawStreamLines(QMatrix4x4 mvpMatrix) -> void {
  shaderProgram.link();
  // Tell OpenGL to use the shader program of this class.
  shaderProgram.bind();

  // Bind the vertex array object that links to the bounding box vertices.
  vertexArrayObject.bind();

  // Set the model-view-projection matrix as a uniform value.
  shaderProgram.setUniformValue("mvpMatrix", mvpMatrix);
  shaderProgram.setUniformValue("isActive", isActive);
  // Issue OpenGL draw commands.
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
  f->glLineWidth(1);
  f->glDrawArrays(GL_LINES, 0, streamLines.size());

  // Release objects until next render cycle.
  vertexArrayObject.release();
  shaderProgram.release();
}

auto StreamLinesRenderer::setMaxSteps() -> void {
  streamLinesMapper->setMaxSteps();
  maxSteps = streamLinesMapper->getDimension() - 1;
  restartPathLines();
}
auto StreamLinesRenderer::setMapper(StreamLinesMapper *mapper) -> void {
  streamLinesMapper = mapper;
}

auto StreamLinesRenderer::updateStreamLines() -> void {
  // streamLines =
  //     streamLinesMapper->computeStreamLines({QVector3D(15.5, 15.5, 15.5)});

  streamLines = streamLinesMapper->computeStreamLines(seeds);

  if (isShift) {
    if (streamLinesMapper->getFrame() % interval == 0)
      seeds = streamLinesMapper->shiftSeeds(seeds);
  }

  initStreamLines();
}

auto StreamLinesRenderer::initOpenGLShaders() -> void {
  if (!shaderProgram.addShaderFromSourceFile(
          QOpenGLShader::Vertex, "../glsl/lines_vshader_streamRenderer.glsl")) {
    std::cout << "Vertex shader error:\n"
              << shaderProgram.log().toStdString() << "\n"
              << std::flush;
    return;
  }

  if (!shaderProgram.addShaderFromSourceFile(
          QOpenGLShader::Fragment,
          "../glsl/lines_fshader_streamRenderer.glsl")) {
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
auto StreamLinesRenderer::initStreamLines() -> void {
  // Vertices of a unit cube that represents the bounding box.

  // Create vertex buffer and upload vertex data to buffer.
  vertexBuffer.bind();
  vertexBuffer.allocate(streamLines.data(),
                        streamLines.size() * 3 * sizeof(float));
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
auto StreamLinesRenderer::setCurrentIntegration(int direction) -> void {
  streamLinesMapper->setCurrentIntegration(direction);
  updateStreamLines();
}

auto StreamLinesRenderer::toggleStreamEdit(bool state) -> void {
  isActive = state && !isActive;
}

auto StreamLinesRenderer::getStreamState() -> bool { return isActive; }

auto StreamLinesRenderer::getTValue() -> float {
  return streamLinesMapper->getTValue();
}

auto StreamLinesRenderer::getIntegration() -> Integration {
  return streamLinesMapper->getIntegration();
}

auto StreamLinesRenderer::getPathLinesInterval() -> int {
  return streamLinesMapper->getPathLinesInterval();
}

auto StreamLinesRenderer::getShiftingSeedsInterval() -> int { return interval; }

auto StreamLinesRenderer::getShiftingState() -> bool { return isShift; }

auto StreamLinesRenderer::getPathState() -> bool {
  return streamLinesMapper->getPathState();
}

auto StreamLinesRenderer::setTValue(float T) -> void {
  if (streamLinesMapper->setTValue(T)) {
    updateStreamLines();
  }
}
