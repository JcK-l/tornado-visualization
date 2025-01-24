//
// Created by Joshua Lowe on 19.06.22.
//

#ifndef CODE4_HORIZONTALSLICETOCONTOURLINEMAPPER_H
#define CODE4_HORIZONTALSLICETOCONTOURLINEMAPPER_H

#include <QVector3D>
#include <QVector>
#include <mutex>
#include <tuple>

#include "FlowDataSource.h"

struct Point {
  void setLocation(int j, int i) {
    this->x = j;
    this->y = i;
  };
  int x;
  int y;
  float value;
};

class HorizontalSliceToContourLineMapper {
 public:
  HorizontalSliceToContourLineMapper();
  explicit HorizontalSliceToContourLineMapper(FlowDataSource*);
  virtual ~HorizontalSliceToContourLineMapper() = default;

  auto setMaxSteps() -> void;
  auto setWindComponent(int) -> void;
  auto getDimension() -> int;
  auto setDataSource(FlowDataSource*) -> void;
  auto mapSliceToContourLineSegments(int, float) -> QVector<QVector3D>;

 private:
  auto helperFunction(int, float, int) -> void;
  auto interpolation(Point, Point, float) -> std::tuple<float, float>;
  auto countSetBits(unsigned int) -> int;
  static auto power2(unsigned int) -> int;
  static auto rotationLeft(int) -> int;
  static auto rotationRight(int) -> int;

  unsigned int maxThreads;
  QVector<QVector3D> points;
  std::mutex pointslock;
  int component;
  int maxSteps;
  FlowDataSource* dataSource;
};

#endif  // CODE4_HORIZONTALSLICETOCONTOURLINEMAPPER_H
