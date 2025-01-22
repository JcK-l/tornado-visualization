//
// Created by Joshua Lowe on 26.06.22.
//

#ifndef CODE5_STREAMLINESMAPPER_H
#define CODE5_STREAMLINESMAPPER_H

#include <QVector3D>

#include "FlowDataSource.h"

struct Point3D {
  Point3D(float x, float y, float z);

  auto setLocation(float x, float y, float z) -> void;
  auto setValue(const std::function<float(int, int, int, int)>& func) -> void;
  QVector3D position;
  QVector3D value;
};

struct Cube {
  Cube(QVector3D initialPoint, FlowDataSource* source, int);
  FlowDataSource* dataSource;

  Point3D p1, p2, p3, p4, p5, p6, p7, p8;
  float x1, x2, y1, y2, z1, z2;
  int maxSteps;
};

enum Integration { Euler, Kutta2, Kutta4 };

class StreamLinesMapper {
 public:
  StreamLinesMapper();
  explicit StreamLinesMapper(FlowDataSource*);
  virtual ~StreamLinesMapper() = default;

  auto getPathLinesInterval() -> int;
  auto clearPathLinesSeeds() -> void;
  auto togglePathLines(bool) -> void;
  auto setPathLinesInterval(int) -> void;
  auto getTValue() -> float;
  auto getPathState() -> bool;
  auto setTValue(float) -> bool;
  auto getFrame() -> int;
  auto getIntegration() -> Integration;
  auto setMaxSteps() -> void;
  auto getDimension() -> int;
  auto setDataSource(FlowDataSource*) -> void;
  auto computeStreamLines(const std::vector<QVector3D>&)
      -> std::vector<QVector3D>;
  auto setCurrentIntegration(int direction) -> void;
  auto shiftSeeds(std::vector<QVector3D>) -> std::vector<QVector3D>;

 private:
  auto helperFunctionStreamLines(const std::vector<QVector3D>&, int) -> void;
  auto helperFunctionPathLines(int) -> void;
  auto attachPathLines(const std::vector<QVector3D>&) -> void;
  auto currentIntegration(QVector3D, QVector3D) -> QVector3D;
  auto trilinearInterpolation(QVector3D) -> QVector3D;
  auto calculateGridPoint(QVector3D) -> QVector3D;
  auto eulerIntegration(QVector3D, QVector3D) -> QVector3D;
  auto rungeKutta2Integration(QVector3D, QVector3D) -> QVector3D;
  auto rungeKutta4Integration(QVector3D, QVector3D) -> QVector3D;

  int pathLinesInterval;

  std::vector<QVector3D> pathLinesSeeds;
  std::vector<QVector3D> tempSeeds;
  std::vector<QVector3D> pathLinesResult;
  std::mutex pathlock;
  std::mutex templock;

  std::vector<QVector3D> streamLines;
  bool isStreamLines;
  unsigned int maxThreads;
  Integration integrationState;
  std::mutex streamlock;
  float t;
  int maxSteps;
  FlowDataSource* dataSource;
};

#endif  // CODE5_STREAMLINESMAPPER_H
