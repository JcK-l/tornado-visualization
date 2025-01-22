//
// Created by Joshua Lowe on 05.05.22.
//

#ifndef UNTITLED_HORIZONTALSLICETOIMAGEMAPPER_H
#define UNTITLED_HORIZONTALSLICETOIMAGEMAPPER_H

#include <QImage>

#include "FlowDataSource.h"

enum Mode { Default, Data };

class HorizontalSliceToImageMapper {
 public:
  HorizontalSliceToImageMapper();
  explicit HorizontalSliceToImageMapper(FlowDataSource*);
  explicit HorizontalSliceToImageMapper(QString);
  HorizontalSliceToImageMapper(FlowDataSource*, QString);
  virtual ~HorizontalSliceToImageMapper() = default;

  auto getMode() -> Mode;
  auto getWindComponent() -> QString;
  auto setWindComponent(int) -> void;
  auto setDataSource(FlowDataSource*) -> void;
  auto getDimension() -> int;
  auto setImageSource(QString) -> void;
  auto mapSliceToImage(int) -> QImage;
  auto mapSliceToImageHCL(int) -> QImage;
  auto getImage() -> QImage;
  auto createImage(int) -> QImage;
  auto setMode(Mode) -> void;
  auto toggleHCL(bool) -> void;
  auto isHCL() -> bool;

 private:
  auto mapToRange(float, float, float, int, int) -> float;
  bool isActive;
  Mode mode;
  int component;
  QString pathToSource;
  FlowDataSource* dataSource;
};

#endif  // UNTITLED_HORIZONTALSLICETOIMAGEMAPPER_H
