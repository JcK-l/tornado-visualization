#include "ContourRendererGLSL.h"

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <fstream>
#include <iostream>

ContourRendererGLSL::ContourRendererGLSL(FlowDataSource* source)
    : HorizontalContourLinesRenderer() {
  datasource = source;
  maxSteps = datasource->getDimension() - 1;
  isoValues.append(0);
  vertexBuffer.create();  // make sure to destroy in destructor!
  initOpenGLShaders();
  updateIso();
}

auto ContourRendererGLSL::drawContour(QMatrix4x4 mvpMatrix) -> void {
  shaderProgram.link();
  // Tell OpenGL to use the shader program of this class.
  shaderProgram.bind();

  // Bind the vertex array object that links to the bounding box vertices.
  vertexArrayObject.bind();

  // Set the model-view-projection matrix as a uniform value.
  shaderProgram.setUniformValue("mvpMatrix", mvpMatrix);
  shaderProgram.setUniformValue("currentStep",
                                (float)currentStep / (float)maxSteps);
  shaderProgram.setUniformValue("component", component);

  // Issue OpenGL draw commands.
  QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

  f->glLineWidth(1);

  for (int i = 0; i < isoValues.size(); i++) {
    if (i == currentActiveValue && isActive) {
      shaderProgram.setUniformValue("isActive", true);
    } else {
      shaderProgram.setUniformValue("isActive", false);
    }
    shaderProgram.setUniformValue("c", isoValues.at(i));
    f->glDrawArrays(GL_LINES_ADJACENCY, 0, input.size() / 2);
  }

  // Release objects until next render cycle.
  vertexArrayObject.release();
  shaderProgram.release();
}

auto ContourRendererGLSL::initOpenGLShaders() -> void {
  if (!shaderProgram.addShaderFromSourceFile(
          QOpenGLShader::Vertex, "../glsl/marching_squares_vshader.glsl")) {
    std::cout << "Vertex shader error:\n"
              << shaderProgram.log().toStdString() << "\n"
              << std::flush;
    return;
  }

  if (!shaderProgram.addShaderFromSourceFile(
          QOpenGLShader::Fragment, "../glsl/marching_squares_fshader.glsl")) {
    std::cout << "Fragment shader error:\n"
              << shaderProgram.log().toStdString() << "\n"
              << std::flush;
    return;
  }

  if (!shaderProgram.addShaderFromSourceFile(
          QOpenGLShader::Geometry, "../glsl/marching_squares_gshader.glsl")) {
    std::cout << "Geometry shader error:\n"
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

auto ContourRendererGLSL::initContourLines() -> void {
  // Vertices of a unit cube that represents the bounding box.

  // Create vertex buffer and upload vertex data to buffer.
  vertexBuffer.bind();
  vertexBuffer.allocate(input.data(), input.size() * 3 * sizeof(float));
  vertexBuffer.release();

  // Store the information OpenGL needs for rendering the vertex
  // buffer in a "vertex array object". This can easily be bound
  // to the OpenGL pipeline during rendering.
  QOpenGLVertexArrayObject::Binder vaoBinder(&vertexArrayObject);
  if (vertexArrayObject.isCreated()) {
    vertexBuffer.bind();
    shaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));
    shaderProgram.enableAttributeArray(0);

    shaderProgram.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3,
                                     6 * sizeof(float));
    shaderProgram.enableAttributeArray(1);
    vertexBuffer.release();
  }
}

auto ContourRendererGLSL::isGLSL() -> bool { return true; }

auto ContourRendererGLSL::setMaxSteps() -> void {
  maxSteps = datasource->getDimension() - 1;
  currentStep =
      ((float)currentStep / (float)maxSteps > 1) ? maxSteps : currentStep;

  updateIso();
}

auto ContourRendererGLSL::updateIso() -> void {
  input.clear();
  datasource->createData();
  for (int i = 0; i < maxSteps; i++) {
    for (int j = 0; j < maxSteps; j++) {
      input.append({(float)j / (float)maxSteps, (float)i / (float)maxSteps, 0});
      input.append({datasource->getDataValue(j, i, currentStep, 0),
                    datasource->getDataValue(j, i, currentStep, 1),
                    datasource->getDataValue(j, i, currentStep, 2)});

      input.append(
          {(float)(j + 1) / (float)maxSteps, (float)i / (float)maxSteps, 0});
      input.append({datasource->getDataValue(j + 1, i, currentStep, 0),
                    datasource->getDataValue(j + 1, i, currentStep, 1),
                    datasource->getDataValue(j + 1, i, currentStep, 2)});

      input.append({(float)(j + 1) / (float)maxSteps,
                    (float)(i + 1) / (float)maxSteps, 0});
      input.append({datasource->getDataValue(j + 1, i + 1, currentStep, 0),
                    datasource->getDataValue(j + 1, i + 1, currentStep, 1),
                    datasource->getDataValue(j + 1, i + 1, currentStep, 2)});

      input.append(
          {(float)j / (float)maxSteps, (float)(i + 1) / (float)maxSteps, 0});
      input.append({datasource->getDataValue(j, i + 1, currentStep, 0),
                    datasource->getDataValue(j, i + 1, currentStep, 1),
                    datasource->getDataValue(j, i + 1, currentStep, 2)});
    }
  }
  initContourLines();
}

auto ContourRendererGLSL::setWindComponent(int ic) -> void { component = ic; }
