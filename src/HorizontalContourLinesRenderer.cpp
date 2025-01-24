//
// Created by Joshua Lowe on 19.06.22.
//

#include "HorizontalContourLinesRenderer.h"

#include <QDir>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <iostream>

HorizontalContourLinesRenderer::HorizontalContourLinesRenderer()
    : vertexBuffer(QOpenGLBuffer::VertexBuffer),
      currentStep(0),
      isActive(false),
      isoValues(QVector<float>()),
      currentActiveValue(0) {}

HorizontalContourLinesRenderer::HorizontalContourLinesRenderer(
    HorizontalSliceToContourLineMapper* mapper)
    : vertexBuffer(QOpenGLBuffer::VertexBuffer),
      maxSteps(mapper->getDimension() - 1),
      currentStep(0),
      contourMapper(mapper),
      isActive(false),
      fragmentColor(0b1111),
      isoValues(QVector<float>()),
      currentActiveValue(0),
      isoLines(QVector<QVector3D>()),
      isoLinesSizes(QVector<GLint>()) {
  initOpenGLShaders();
  isoLinesSizes.reserve(3);
  isoValues.append(0);
  vertexBuffer.create();  // make sure to destroy in destructor!
  updateIso();
}

HorizontalContourLinesRenderer::~HorizontalContourLinesRenderer() {
  vertexBuffer.destroy();
}

auto HorizontalContourLinesRenderer::isGLSL() -> bool { return false; }

auto HorizontalContourLinesRenderer::drawContour(QMatrix4x4 mvpMatrix) -> void {
  shaderProgram.link();
  // Tell OpenGL to use the shader program of this class.
  shaderProgram.bind();

  // Bind the vertex array object that links to the bounding box vertices.
  vertexArrayObject.bind();

  // Set the model-view-projection matrix as a uniform value.
  shaderProgram.setUniformValue("mvpMatrix", mvpMatrix);
  shaderProgram.setUniformValue("currentStep",
                                (float)currentStep / (float)maxSteps);
  shaderProgram.setUniformValue("isoStateColor", fragmentColor);
  shaderProgram.setUniformValue("currentActiveIso", currentActiveValue);
  shaderProgram.setUniformValueArray("IsoSizes", isoLinesSizes.data(),
                                     MAX_ISO_AMOUNT);

  // Issue OpenGL draw commands.
  QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
  // f->glEnable(GL_BLEND);
  // f->glDepthMask(false);
  // f->glLineWidth(1);
  f->glLineWidth(2);
  f->glDrawArrays(GL_LINES, 0, isoLines.size());

  // Release objects until next render cycle.
  vertexArrayObject.release();
  shaderProgram.release();
}

auto HorizontalContourLinesRenderer::initOpenGLShaders() -> void {
  QString shaderDir = QString::fromUtf8(std::getenv("SHADER_DIR"));
  QString vertexShaderPath =
      SHADER_DIR + QString("lines_vshader_contourRenderer.glsl");
  QString fragmentShaderPath = SHADER_DIR + QString("lines_fshader_contourRenderer.glsl");

  if (!shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                             vertexShaderPath)) {
    std::cout << "Vertex shader error:\n"
              << shaderProgram.log().toStdString() << "\n"
              << std::flush;
    return;
  }

  if (!shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                             fragmentShaderPath)) {
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

auto HorizontalContourLinesRenderer::setMapper(
    HorizontalSliceToContourLineMapper* mapper) -> void {
  contourMapper = mapper;
}

auto HorizontalContourLinesRenderer::moveSlice(int steps) -> void {
  currentStep += steps;

  currentStep = (currentStep < 0) ? 0 : currentStep;
  currentStep = (currentStep > maxSteps) ? maxSteps : currentStep;
  updateIso();
}

auto HorizontalContourLinesRenderer::setMaxSteps() -> void {
  contourMapper->setMaxSteps();
  maxSteps = contourMapper->getDimension() - 1;
  currentStep =
      ((float)currentStep / (float)maxSteps > 1) ? maxSteps : currentStep;

  updateIso();
}

auto HorizontalContourLinesRenderer::setCurrentStep(int step) -> void {
  currentStep = step;
 }

auto HorizontalContourLinesRenderer::initContourLines() -> void {
  // Vertices of a unit cube that represents the bounding box.

  // Create vertex buffer and upload vertex data to buffer.
  for (int i = 0; i < isoLines.size() - 1; ++i) {
    //std::cout << "[" << isoLines.at(i).x() << "," << isoLines.at(i).y() << ","
              //<< isoLines.at(i).z() << "]";
  }
  vertexBuffer.bind();
  vertexBuffer.allocate(isoLines.data(), isoLines.size() * 3 * sizeof(float));
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
auto HorizontalContourLinesRenderer::setWindComponent(int ic) -> void {
  contourMapper->setWindComponent(ic);
  updateIso();
}
auto HorizontalContourLinesRenderer::toggleIsoEdit(bool state) -> void {
  isActive = state && !isActive;
  if (isActive) {
    fragmentColor = 0b1001;
  } else {
    fragmentColor = 0b1111;
  }
}
auto HorizontalContourLinesRenderer::getIsoState() -> bool { return isActive; }

auto HorizontalContourLinesRenderer::deleteIsoLine() -> void {
  if (isoValues.size() == 1) return;
  isoValues.removeLast();
  currentActiveValue = (currentActiveValue > isoValues.size() - 1)
                           ? isoValues.size() - 1
                           : currentActiveValue;
  updateIso();
}
auto HorizontalContourLinesRenderer::updateIso() -> void {
  isoLines.clear();
  isoLinesSizes.clear();
  for (auto& isoValue : isoValues) {
    isoLines.append(
        contourMapper->mapSliceToContourLineSegments(currentStep, isoValue));
    isoLinesSizes.append(isoLines.size());
  }

  initContourLines();
}
auto HorizontalContourLinesRenderer::addIsoLine() -> void {
  switch (isoValues.size()) {
    case 1:
      isoValues.append(0.1);
      break;
    case 2:
      isoValues.append(-0.1);
      break;
    case 3:
      isoValues.append(0.15);
      break;
    case 4:
      isoValues.append(-0.15);
      break;
  }
  updateIso();
}
auto HorizontalContourLinesRenderer::setActiveIso(int steps) -> void {
  currentActiveValue += steps;

  currentActiveValue = (currentActiveValue < 0) ? 0 : currentActiveValue;
  currentActiveValue = (currentActiveValue > isoValues.size() - 1)
                           ? isoValues.size() - 1
                           : currentActiveValue;
}
auto HorizontalContourLinesRenderer::moveActiveIso(float steps) -> void {
  isoValues[currentActiveValue] += steps;
  updateIso();
}
auto HorizontalContourLinesRenderer::getValuesArray() -> QVector<float> {
  return isoValues;
}
auto HorizontalContourLinesRenderer::getCurrentActiveValue() -> int {
  return currentActiveValue;
}
