#pragma once

#include "HorizontalContourLinesRenderer.h"

class ContourRendererGLSL : public HorizontalContourLinesRenderer {
 public:
  ContourRendererGLSL(FlowDataSource*);
  ~ContourRendererGLSL() = default;

  // Draw the bounding box to the current OpenGL viewport.
  auto drawContour(QMatrix4x4 mvpMatrix) -> void override;
  auto setWindComponent(int ic) -> void override;
  auto setMaxSteps() -> void override;
  auto isGLSL() -> bool override;

 protected:
  auto initOpenGLShaders() -> void override;
  auto initContourLines() -> void override;
  auto updateIso() -> void override;

 private:
  FlowDataSource* source;
  int component = 0;
  QVector<QVector3D> input;
  FlowDataSource* datasource;
};
