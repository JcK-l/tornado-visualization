//
// Created by Joshua Lowe on 19.06.22.
//

#include "HorizontalSliceToContourLineMapper.h"

#include <cmath>
#include <thread>
#include <utility>

#include "FlowDataSource.h"

HorizontalSliceToContourLineMapper::HorizontalSliceToContourLineMapper()
    : component(0), maxSteps(31), maxThreads(5) {}

HorizontalSliceToContourLineMapper::HorizontalSliceToContourLineMapper(
    FlowDataSource* source)
    : HorizontalSliceToContourLineMapper() {
  setDataSource(source);
  maxSteps = source->getDimension() - 1;
}

auto HorizontalSliceToContourLineMapper::setMaxSteps() -> void {
  maxSteps = dataSource->getDimension() - 1;
}

auto HorizontalSliceToContourLineMapper::setDataSource(FlowDataSource* source)
    -> void {
  dataSource = source;
}

auto HorizontalSliceToContourLineMapper::getDimension() -> int {
  return dataSource->getDimension();
}

auto HorizontalSliceToContourLineMapper::interpolation(Point p, Point pd,
                                                       float c)
    -> std::tuple<float, float> {
  if (p.x - pd.x != 0) {
    return {((c - p.value) *
             ((float)pd.x / (float)maxSteps - (float)p.x / (float)maxSteps) /
             (pd.value - p.value)) +
                (float)p.x / (float)maxSteps,
            (float)p.y / (float)maxSteps};
  } else {
    return {(float)p.x / (float)maxSteps,
            ((c - p.value) *
             ((float)pd.y / (float)maxSteps - (float)p.y / (float)maxSteps) /
             (pd.value - p.value)) +
                (float)p.y / (float)maxSteps};
  }
}

auto HorizontalSliceToContourLineMapper::rotationLeft(int in) -> int {
  int result = in << 1;
  result = (result & 0x10) ? result - 16 + 1 : result;
  return result;
}

auto HorizontalSliceToContourLineMapper::rotationRight(int in) -> int {
  int result = (in & 0x01) ? in + 16 : in;
  return result >> 1;
}

auto HorizontalSliceToContourLineMapper::countSetBits(unsigned int in) -> int {
  int count = 0;
  for (int i = 0; i < floor(log2(in) + 1); i++) {
    count = ((power2(i) & in) > 0) ? count + 1 : count;
  }
  return count;
}

auto HorizontalSliceToContourLineMapper::power2(unsigned int in) -> int {
  int result = 1;
  for (int i = 0; i < in; i++) {
    result *= 2;
  }
  return result;
}

auto HorizontalSliceToContourLineMapper::helperFunction(int iz, float c, int ID)
    -> void {
  Point p1, p2, p3, p4;
  p1 = Point();
  p2 = Point();
  p3 = Point();
  p4 = Point();

  int bitMap;
  auto getPoint = [&](int num) -> Point {
    switch (num) {
      case 0x1:
        return p1;
      case 0x2:
        return p2;
      case 0x4:
        return p3;
      case 0x8:
        return p4;
      default:
        return {};
    }
  };

  for (int i = 0; i < maxSteps; i++) {
    if (i % (maxThreads - 1) != ID) continue;
    for (int j = 0; j < maxSteps; j++) {
      p1.setLocation(j, i);
      p2.setLocation(j + 1, i);
      p3.setLocation(j + 1, i + 1);
      p4.setLocation(j, i + 1);

      p1.value = dataSource->getDataValue(p1.x, p1.y, iz, component);
      p2.value = dataSource->getDataValue(p2.x, p2.y, iz, component);
      p3.value = dataSource->getDataValue(p3.x, p3.y, iz, component);
      p4.value = dataSource->getDataValue(p4.x, p4.y, iz, component);

      bitMap = 0x0;
      bitMap = (p1.value > c) ? bitMap | 0x01 : bitMap;
      bitMap = (p2.value > c) ? bitMap | 0x02 : bitMap;
      bitMap = (p3.value > c) ? bitMap | 0x04 : bitMap;
      bitMap = (p4.value > c) ? bitMap | 0x08 : bitMap;

      std::tuple<float, float> iso1, iso2;

      if (bitMap == 0x5 || bitMap == 0xA) {
        std::tuple<float, float> iso3, iso4;

        iso1 = interpolation(getPoint(rotationLeft(bitMap & 0x3)),
                             getPoint(bitMap & 0x3), c);
        iso2 = interpolation(getPoint(rotationRight(bitMap & 0x3)),
                             getPoint(bitMap & 0x3), c);
        iso3 = interpolation(getPoint(rotationLeft(bitMap & 0xC)),
                             getPoint(bitMap & 0xC), c);
        iso4 = interpolation(getPoint(rotationRight(bitMap & 0xC)),
                             getPoint(bitMap & 0xC), c);
        float asymptote = ((p4.value * p2.value) - (p3.value * p1.value)) /
                          (p1.value - p3.value - p4.value + p2.value);
        {
          std::lock_guard<std::mutex> lock(pointslock);
          if (asymptote < c) {
            points.push_back(
                QVector3D(std::get<0>(iso1), std::get<1>(iso1), 0));
            points.push_back(
                QVector3D(std::get<0>(iso2), std::get<1>(iso2), 0));
            points.push_back(
                QVector3D(std::get<0>(iso3), std::get<1>(iso3), 0));
            points.push_back(
                QVector3D(std::get<0>(iso4), std::get<1>(iso4), 0));
          } else {
            points.push_back(
                QVector3D(std::get<0>(iso2), std::get<1>(iso2), 0));
            points.push_back(
                QVector3D(std::get<0>(iso3), std::get<1>(iso3), 0));
            points.push_back(
                QVector3D(std::get<0>(iso1), std::get<1>(iso1), 0));
            points.push_back(
                QVector3D(std::get<0>(iso4), std::get<1>(iso4), 0));
          }
        }
        continue;
      }

      switch (countSetBits(bitMap)) {
        case 1:
          iso1 = interpolation(getPoint(rotationLeft(bitMap)), getPoint(bitMap),
                               c);
          iso2 = interpolation(getPoint(rotationRight(bitMap)),
                               getPoint(bitMap), c);
          break;
        case 2:
          iso1 = interpolation(getPoint(rotationLeft(bitMap) & ~bitMap),
                               getPoint(rotationLeft(bitMap) & bitMap), c);
          iso2 = interpolation(getPoint(rotationRight(bitMap) & ~bitMap),
                               getPoint(rotationRight(bitMap) & bitMap), c);
          break;
        case 3:
          iso1 = interpolation(getPoint(bitMap ^ 0xF),
                               getPoint(rotationLeft(bitMap ^ 0xF)), c);
          iso2 = interpolation(getPoint(bitMap ^ 0xF),
                               getPoint(rotationRight(bitMap ^ 0xF)), c);
          break;
        default:
          continue;
      }
      {
        std::lock_guard<std::mutex> lock(pointslock);
        points.push_back(QVector3D(std::get<0>(iso1), std::get<1>(iso1), 0));
        points.push_back(QVector3D(std::get<0>(iso2), std::get<1>(iso2), 0));
      }
    }
  }
}

auto HorizontalSliceToContourLineMapper::mapSliceToContourLineSegments(int z,
                                                                       float c)
    -> QVector<QVector3D> {
  points.clear();
  dataSource->createData();
  std::vector<std::thread> threads;
  for (int i = 0; i < (maxThreads - 1); i++) {
    threads.emplace_back(&HorizontalSliceToContourLineMapper::helperFunction,
                         this, z, c, i);
  }
  for (std::thread& thread : threads) {
    thread.join();
  }
  return points;
}

auto HorizontalSliceToContourLineMapper::setWindComponent(int ic) -> void {
  component = ic;
}
