//
// Created by Joshua Lowe on 15.05.22.
//
#include "HorizontalSliceRenderer.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <iostream>

HorizontalSliceRenderer::HorizontalSliceRenderer()
    : HorizontalSliceRenderer(new HorizontalSliceToImageMapper()) {}

HorizontalSliceRenderer::HorizontalSliceRenderer(
    HorizontalSliceToImageMapper *mapper)
    : vertexBuffer(QOpenGLBuffer::VertexBuffer),
      maxSteps(mapper->getDimension() - 1),
      currentStep(0),
      imageMapper(mapper),
      texture(new QOpenGLTexture(QOpenGLTexture::Target2D)),
      sliceCorners{{1, 1, 0}, {1, 0, 0}, {0, 1, 0},
                   {0, 1, 0}, {0, 0, 0}, {1, 0, 0}} {
  initOpenGLShaders();
  initHorizontalSlice();
}

HorizontalSliceRenderer::~HorizontalSliceRenderer() {
  vertexBuffer.destroy();
  texture->destroy();
}

auto HorizontalSliceRenderer::isHCL() -> bool { return imageMapper->isHCL(); }

auto HorizontalSliceRenderer::drawImage(QMatrix4x4 mvpMatrix) -> void {
  shaderProgram.link();
  // Tell OpenGL to use the shader program of this class.
  shaderProgram.bind();

  // Bind the vertex array object that links to the bounding box vertices.
  vertexArrayObject.bind();

  const int textureUnit = 0;  // select a texture unit
  texture->bind(textureUnit);
  shaderProgram.setUniformValue("colorMappingTexture", textureUnit);
  // Set the model-view-projection matrix as a uniform value.
  shaderProgram.setUniformValue("mvpMatrix", mvpMatrix);
  shaderProgram.setUniformValue("currentStep",
                                (float)currentStep / (float)maxSteps);

  // Issue OpenGL draw commands.
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
  f->glLineWidth(2);
  f->glDrawArrays(GL_TRIANGLES, 0, 6);

  // Release objects until next render cycle.
  vertexArrayObject.release();
  shaderProgram.release();
}

auto HorizontalSliceRenderer::initOpenGLShaders() -> void {
  if (!shaderProgram.addShaderFromSourceFile(
          QOpenGLShader::Vertex, "../glsl/lines_vshader_imageRenderer.glsl")) {
    std::cout << "Vertex shader error:\n"
              << shaderProgram.log().toStdString() << "\n"
              << std::flush;
    return;
  }

  if (!shaderProgram.addShaderFromSourceFile(
          QOpenGLShader::Fragment,
          "../glsl/lines_fshader_imageRenderer.glsl")) {
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

auto HorizontalSliceRenderer::setMapper(HorizontalSliceToImageMapper *mapper)
    -> void {
  imageMapper = mapper;
}

auto HorizontalSliceRenderer::moveSlice(int steps) -> void {
  currentStep += steps;

  currentStep = (currentStep < 0) ? 0 : currentStep;
  currentStep = (currentStep > maxSteps) ? maxSteps : currentStep;

  updateTexture();
}

auto HorizontalSliceRenderer::setMaxSteps() -> void {
  maxSteps = imageMapper->getDimension() - 1;
  currentStep =
      ((float)currentStep / (float)maxSteps > 1) ? maxSteps : currentStep;

  updateTexture();
}

auto HorizontalSliceRenderer::setMode(Mode inMode) -> void {
  imageMapper->setMode(inMode);
  updateTexture();
}

auto HorizontalSliceRenderer::newTexture() -> void {
  imageMapper->setImageSource("../j.jpg");
  img = imageMapper->createImage(currentStep);  // some image
  texture->create();
  texture->setWrapMode(QOpenGLTexture::ClampToEdge);
  texture->setData(img);
}

auto HorizontalSliceRenderer::initHorizontalSlice() -> void {
  // Vertices of a unit cube that represents the bounding box.
  newTexture();

  // Create vertex buffer and upload vertex data to buffer.
  vertexBuffer.create();  // make sure to destroy in destructor!
  vertexBuffer.bind();
  vertexBuffer.allocate(sliceCorners, 18 * sizeof(float));
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
auto HorizontalSliceRenderer::setWindComponent(int ic) -> void {
  imageMapper->setWindComponent(ic);
  updateTexture();
}

auto HorizontalSliceRenderer::getWindComponent() -> QString {
  return imageMapper->getWindComponent();
}
auto HorizontalSliceRenderer::getSteps() -> int { return currentStep; }

auto HorizontalSliceRenderer::getMode() -> Mode {
  return imageMapper->getMode();
}

auto HorizontalSliceRenderer::toggleHCL(bool active) -> void {
  imageMapper->toggleHCL(active);
  updateTexture();
}

auto HorizontalSliceRenderer::updateTexture() -> void {
  texture->destroy();
  newTexture();
}
