//
// Created by Joshua Lowe on 26.06.22.
//

#include "StreamLinesMapper.h"

#include <QVector>
#include <algorithm>
#include <iostream>
#include <thread>

StreamLinesMapper::StreamLinesMapper()
    : maxSteps(31),
      integrationState(Euler),
      t(1),
      maxThreads(std::thread::hardware_concurrency()),
      isStreamLines(true),
      pathLinesInterval(5) {}

StreamLinesMapper::StreamLinesMapper(FlowDataSource* source)
    : StreamLinesMapper() {
  setDataSource(source);
  maxSteps = source->getDimension() - 1;
}

auto StreamLinesMapper::setMaxSteps() -> void {
  maxSteps = dataSource->getDimension() - 1;
}

auto StreamLinesMapper::setDataSource(FlowDataSource* source) -> void {
  dataSource = source;
}

auto StreamLinesMapper::togglePathLines(bool state) -> void {
  isStreamLines = state && !isStreamLines;
}

auto StreamLinesMapper::setPathLinesInterval(int step) -> void {
  pathLinesInterval =
      (pathLinesInterval + step < 1) ? 1 : pathLinesInterval + step;
}

auto StreamLinesMapper::getFrame() -> int { return dataSource->getFrame(); }

auto StreamLinesMapper::helperFunctionPathLines(int ID) -> void {
  std::vector<QVector3D> temp;
  for (int i = 0; i < pathLinesSeeds.size(); i++) {
    if (i % (maxThreads - 1) != ID) continue;
    QVector3D currentLocation = pathLinesSeeds.at(i);
    QVector3D prevLocation;

    auto [success1, seedValue] = trilinearInterpolation(currentLocation);
    if (!success1) exit(1);

    prevLocation = currentLocation;

    InterpolationResult location =
        currentIntegration(currentLocation, seedValue);
    if (!location.success) continue;

    currentLocation = location.value;
    InterpolationResult seed = trilinearInterpolation(currentLocation);
    if (!seed.success) continue;
    
    seedValue = seed.value;
    {
      std::lock_guard<std::mutex> lock(templock);
      tempSeeds.push_back(currentLocation);
    }

    QVector3D direction = currentLocation - prevLocation;
    QVector3D tempValue = (currentLocation + QVector3D(1, 0, 0)) - prevLocation;
    direction.normalize();
    QVector3D base = prevLocation;  //+ (0.12 * direction);
    QVector3D direction2 = QVector3D::crossProduct(direction, tempValue);
    direction2.normalize();
    // QVector3D base2 = base + (0.04 * direction2);
    QVector3D base3 = base + (-0.04 * direction2);
    QVector3D direction3 = QVector3D::crossProduct(direction, direction2);
    direction3.normalize();
    QVector3D base4 = base + (0.04 * direction3);
    QVector3D base5 = base + (-0.04 * direction3);
    temp.push_back(prevLocation / maxSteps);
    temp.push_back(currentLocation / maxSteps);

    // pathLinesResult.push_back(base2 / maxSteps);
    temp.push_back(base3 / maxSteps);
    temp.push_back(prevLocation / maxSteps);

    temp.push_back(base4 / maxSteps);
    temp.push_back(base5 / maxSteps);

    // pathLinesResult.push_back(base2 / maxSteps);
    // pathLinesResult.push_back(currentLocation / maxSteps);

    temp.push_back(base3 / maxSteps);
    temp.push_back(currentLocation / maxSteps);

    temp.push_back(base4 / maxSteps);
    temp.push_back(currentLocation / maxSteps);

    temp.push_back(base5 / maxSteps);
    temp.push_back(currentLocation / maxSteps);
  }
  {
    std::lock_guard<std::mutex> lock(pathlock);
    pathLinesResult.insert(std::end(pathLinesResult), std::begin(temp),
                           std::end(temp));
  }
}

auto StreamLinesMapper::helperFunctionStreamLines(
    const std::vector<QVector3D>& seeds, int ID) -> void {
  std::vector<QVector3D> temp;
  for (int i = 0; i < seeds.size(); i++) {
    if (i % (maxThreads - 1) != ID) continue;
    QVector3D currentLocation = seeds.at(i);
    QVector3D prevLocation;

    auto [success, seedValue] = trilinearInterpolation(currentLocation);
    if (!success) exit(1);

    while (true) {
      prevLocation = currentLocation;

      InterpolationResult location =
          currentIntegration(currentLocation, seedValue);
      if (!location.success) break;
      currentLocation = location.value;
      InterpolationResult seed = trilinearInterpolation(currentLocation);
      if (!seed.success) break;
      seedValue = seed.value;

      temp.push_back(prevLocation / maxSteps);
      temp.push_back(currentLocation / maxSteps);
    }
  }
  {
    std::lock_guard<std::mutex> lock(streamlock);
    streamLines.insert(std::end(streamLines), std::begin(temp), std::end(temp));
  }
}

auto StreamLinesMapper::shiftSeeds(std::vector<QVector3D> seeds)
    -> std::vector<QVector3D> {
  std::vector<QVector3D> temp;
  for (int i = 0; i < seeds.size(); i++) {
    QVector3D currentLocation = seeds.at(i);

    auto [success1, seedValue] = trilinearInterpolation(currentLocation);
    if (!success1) exit(1);

    auto location =
        currentIntegration(currentLocation, seedValue);
    if (!location.success) continue;
    currentLocation = location.value;

    auto seed = trilinearInterpolation(currentLocation);
    if (!seed.success) continue;
    seedValue = seed.value;

    temp.push_back(currentLocation);
  }
  return temp;
}

auto StreamLinesMapper::clearPathLinesSeeds() -> void {
  pathLinesSeeds.clear();
}

auto StreamLinesMapper::computeStreamLines(const std::vector<QVector3D>& seeds)
    -> std::vector<QVector3D> {
  streamLines.clear();
  pathLinesResult.clear();
  dataSource->createData();
  if (!isStreamLines) attachPathLines(seeds);
  std::vector<std::thread> threads;
  for (int i = 0; i < (maxThreads - 1); i++) {
    if (isStreamLines) {
      threads.emplace_back(&StreamLinesMapper::helperFunctionStreamLines, this,
                           seeds, i);
    } else {
      threads.emplace_back(&StreamLinesMapper::helperFunctionPathLines, this,
                           i);
    }
  }
  for (std::thread& thread : threads) {
    thread.join();
  }
  threads.clear();

  if (isStreamLines) return streamLines;
  pathLinesSeeds = std::move(tempSeeds);
  return pathLinesResult;
}

auto StreamLinesMapper::attachPathLines(const std::vector<QVector3D>& seeds)
    -> void {
  int frame = dataSource->getFrame();

  if (frame % pathLinesInterval == 0) {
    pathLinesSeeds.insert(std::end(pathLinesSeeds), std::begin(seeds),
                          std::end(seeds));
  }
}

auto StreamLinesMapper::getPathLinesInterval() -> int {
  return pathLinesInterval;
}

auto StreamLinesMapper::trilinearInterpolation(QVector3D seed)
    -> InterpolationResult {
  QVector3D gridPoint = calculateGridPoint(seed);
  if (std::max({gridPoint.x(), gridPoint.y(), gridPoint.z()}) > maxSteps - 1 ||
      std::min({gridPoint.x(), gridPoint.y(), gridPoint.z()}) < 0)
    return {false, QVector3D(0, 0, 0)};

  Cube cube = Cube(gridPoint, dataSource, maxSteps);

  QVector3D firstInterpolation =
      1 / (cube.x2 - cube.x1) * (cube.y2 - cube.y1) *
      (cube.p1.value * (cube.x2 - seed.x()) * (cube.y2 - seed.y()) +
       cube.p2.value * (seed.x() - cube.x1) * (cube.y2 - seed.y()) +
       cube.p4.value * (cube.x2 - seed.x()) * (seed.y() - cube.y1) +
       cube.p3.value * (seed.x() - cube.x1) * (seed.y() - cube.y1));

  QVector3D secondInterpolation =
      1 / (cube.x2 - cube.x1) * (cube.y2 - cube.y1) *
      (cube.p5.value * (cube.x2 - seed.x()) * (cube.y2 - seed.y()) +
       cube.p6.value * (seed.x() - cube.x1) * (cube.y2 - seed.y()) +
       cube.p8.value * (cube.x2 - seed.x()) * (seed.y() - cube.y1) +
       cube.p7.value * (seed.x() - cube.x1) * (seed.y() - cube.y1));

  QVector3D thirdInterpolation =
      (cube.z2 - seed.z()) / (cube.z2 - cube.z1) * firstInterpolation +
      (seed.z() - cube.z1) / (cube.z2 - cube.z1) * secondInterpolation;
  return {true, thirdInterpolation};
}

auto StreamLinesMapper::calculateGridPoint(QVector3D seed) -> QVector3D {
  float x, y, z;
  x = (seed.x() == maxSteps) ? seed.x() - 0.1 / (float)maxSteps : seed.x();
  y = (seed.y() == maxSteps) ? seed.y() - 0.1 / (float)maxSteps : seed.y();
  z = (seed.z() == maxSteps) ? seed.z() - 0.1 / (float)maxSteps : seed.z();
  return {floor(x), floor(y), floor(z)};
}

auto StreamLinesMapper::getDimension() -> int {
  return dataSource->getDimension();
}

auto StreamLinesMapper::getPathState() -> bool { return !isStreamLines; }

auto StreamLinesMapper::eulerIntegration(QVector3D xt, QVector3D value)
    -> InterpolationResult {
  return {true, xt + (t * value)};
}

auto StreamLinesMapper::rungeKutta2Integration(QVector3D xt, QVector3D value)
    -> InterpolationResult {
  QVector3D deltaX = t * (value);
  QVector3D vMidLoc = xt + (deltaX / 2);
  auto [success, vMidVal] = trilinearInterpolation(vMidLoc);
  return {success, xt + (t * vMidVal)};
}

auto StreamLinesMapper::rungeKutta4Integration(QVector3D xt, QVector3D value)
    -> InterpolationResult {
  QVector3D k1, k2, k3, k4;
  QVector3D k2Loc, k3Loc, k4Loc;
  k1 = t * value;

  k2Loc = xt + (k1 / 2);

  auto [success1, k2Val] = trilinearInterpolation(k2Loc);
  if (!success1) return {false, QVector3D(0, 0, 0)};

  k2 = t * k2Val;

  k3Loc = xt + (k2 / 2);
  auto [success2, k3Val] = trilinearInterpolation(k3Loc);
  if (!success2) return {false, QVector3D(0, 0, 0)};

  k3 = t * k3Val;

  k4Loc = xt + k3;

  auto [success3, k4Val] = trilinearInterpolation(k4Loc);
  if (!success3) return {false, QVector3D(0, 0, 0)};

  k4 = t * k4Val;

  return {true, xt + (k1 / 6) + (k2 / 3) + (k3 / 3) + (k4 / 6)};
}

auto StreamLinesMapper::currentIntegration(QVector3D start, QVector3D value)
    -> InterpolationResult {
  switch (integrationState) {
    case Euler:
      return eulerIntegration(start, value);
    case Kutta2:
      return rungeKutta2Integration(start, value);
    case Kutta4:
      return rungeKutta4Integration(start, value);
    default:
      return eulerIntegration(start, value);
  }
}

auto StreamLinesMapper::setCurrentIntegration(int direction) -> void {
  switch (integrationState) {
    case Euler:
      integrationState = (direction < 0) ? Kutta4 : Kutta2;
      break;
    case Kutta2:
      integrationState = (direction < 0) ? Euler : Kutta4;
      break;
    case Kutta4:
      integrationState = (direction < 0) ? Kutta2 : Euler;
      break;
    default:
      integrationState = (direction < 0) ? Kutta4 : Kutta2;
      break;
  }
}
auto StreamLinesMapper::getTValue() -> float { return t; }
auto StreamLinesMapper::getIntegration() -> Integration {
  return integrationState;
}
auto StreamLinesMapper::setTValue(float T) -> bool {
  float prevT = t;
  t = ((t + T) <= 0.05) ? t : t + T;
  if (prevT == t) return false;
  return true;
}

Point3D::Point3D(float x, float y, float z) : position{x, y, z} {}

auto Point3D::setLocation(float x, float y, float z) -> void {
  position = QVector3D(x, y, z);
}

auto Point3D::setValue(const std::function<float(int, int, int, int)>& func)
    -> void {
  float x, y, z;
  x = func(position.x(), position.y(), position.z(), 0);
  y = func(position.x(), position.y(), position.z(), 1);
  z = func(position.x(), position.y(), position.z(), 2);
  value = QVector3D(x, y, z);
}

Cube::Cube(QVector3D initialPoint, FlowDataSource* source, int steps)
    : p1{initialPoint.x(), initialPoint.y(), initialPoint.z()},
      p2{initialPoint.x() + 1, initialPoint.y(), initialPoint.z()},
      p3{initialPoint.x() + 1, initialPoint.y() + 1, initialPoint.z()},
      p4{initialPoint.x(), initialPoint.y() + 1, initialPoint.z()},
      p5{initialPoint.x(), initialPoint.y(), initialPoint.z() + 1},
      p6{initialPoint.x() + 1, initialPoint.y(), initialPoint.z() + 1},
      p7{initialPoint.x() + 1, initialPoint.y() + 1, initialPoint.z() + 1},
      p8{initialPoint.x(), initialPoint.y() + 1, initialPoint.z() + 1},
      dataSource(source),
      maxSteps(steps) {
  p1.setValue([&](int x, int y, int z, int c) -> float {
    return dataSource->getDataValue(x, y, z, c);
  });
  p2.setValue([&](int x, int y, int z, int c) -> float {
    return dataSource->getDataValue(x, y, z, c);
  });
  p3.setValue([&](int x, int y, int z, int c) -> float {
    return dataSource->getDataValue(x, y, z, c);
  });
  p4.setValue([&](int x, int y, int z, int c) -> float {
    return dataSource->getDataValue(x, y, z, c);
  });
  p5.setValue([&](int x, int y, int z, int c) -> float {
    return dataSource->getDataValue(x, y, z, c);
  });
  p6.setValue([&](int x, int y, int z, int c) -> float {
    return dataSource->getDataValue(x, y, z, c);
  });
  p7.setValue([&](int x, int y, int z, int c) -> float {
    return dataSource->getDataValue(x, y, z, c);
  });
  p8.setValue([&](int x, int y, int z, int c) -> float {
    return dataSource->getDataValue(x, y, z, c);
  });
  this->x1 = p1.position.x();
  this->x2 = p2.position.x();
  this->y1 = p1.position.y();
  this->y2 = p3.position.y();
  this->z1 = p1.position.z();
  this->z2 = p5.position.z();
}
