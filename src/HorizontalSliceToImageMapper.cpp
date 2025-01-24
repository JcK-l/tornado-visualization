//
// Created by Joshua Lowe on 05.05.22.
//

#include "HorizontalSliceToImageMapper.h"

#include <fstream>
#include <iostream>

#include "FlowDataSource.h"

HorizontalSliceToImageMapper::HorizontalSliceToImageMapper()
    : component(0), mode(Default), isActive(true) {}

HorizontalSliceToImageMapper::HorizontalSliceToImageMapper(
    FlowDataSource* source)
    : HorizontalSliceToImageMapper() {
  setDataSource(source);
}

HorizontalSliceToImageMapper::HorizontalSliceToImageMapper(QString ipath)
    : HorizontalSliceToImageMapper() {
  setImageSource(ipath);
}

HorizontalSliceToImageMapper::HorizontalSliceToImageMapper(
    FlowDataSource* source, QString ipath)
    : HorizontalSliceToImageMapper() {
  setDataSource(source);
  setImageSource(ipath);
}

auto HorizontalSliceToImageMapper::setImageSource(QString ipath) -> void {
  pathToSource = ipath;
}

auto HorizontalSliceToImageMapper::setDataSource(FlowDataSource* source)
    -> void {
  dataSource = source;
}

auto HorizontalSliceToImageMapper::getDimension() -> int {
  return dataSource->getDimension();
}

auto HorizontalSliceToImageMapper::mapSliceToImage(int iz) -> QImage {
  dataSource->createData();
  int dimension = dataSource->getDimension();
  float xc;
  QImage image = QImage(dimension, dimension, QImage::Format_RGBA64);
  for (int i = 0; i < dimension; i++) {
    for (int j = 0; j < dimension; j++) {
      xc = dataSource->getDataValue(j, i, iz, component);
      if (xc >= 0) {
        image.setPixelColor(j, i, QColor((int)(xc * 3 * 255), 0, 0));
      } else {
        xc *= -1;
        image.setPixelColor(j, i, QColor(0, 0, (int)(xc * 3 * 255)));
      }
    }
  }
  return image;
}

auto HorizontalSliceToImageMapper::isHCL() -> bool { return isActive; }

auto HorizontalSliceToImageMapper::mapSliceToImageHCL(int iz) -> QImage {
  QVector<QColor> colormap;
  std::ifstream input(DATA_DIR + QString("colormap3.txt").toStdString());
  if (!input) {
    std::cerr << "file not found";
    exit(1);
  }
  std::string line;
  int r, g, b;
  int count = 0;
  while (std::getline(input, line, ',')) {
    switch (count) {
      case 0:
        r = std::stoi(line);
        break;
      case 1:
        g = std::stoi(line);
        break;
      case 2:
        b = std::stoi(line);
        colormap.append(QColor(r, g, b));
        break;
    }
    count = (count + 1) % 3;
  }
  input.close();
  dataSource->createData();
  int dimension = dataSource->getDimension();
  float x, xc, max, min;
  // max =
  //     std::max(dataSource->getMaxValue(component),
  //     dataSource->getMaxBetrag());
  // min = dataSource->getMinValue(component);
  max = 0.4;
  min = -0.4;
  QImage image = QImage(dimension, dimension, QImage::Format_RGBA64);
  for (int i = 0; i < dimension; i++) {
    for (int j = 0; j < dimension; j++) {
      x = dataSource->getDataValue(j, i, iz, component);
      xc = mapToRange(x, min, max, 0, colormap.size() - 1);

      image.setPixelColor(j, i, colormap.at((int)round(xc)));
    }
  }
  return image;
}

auto HorizontalSliceToImageMapper::getImage() -> QImage {
  return QImage(pathToSource).mirrored();
}
auto HorizontalSliceToImageMapper::setWindComponent(int ic) -> void {
  component = ic;
}

auto HorizontalSliceToImageMapper::setMode(Mode inMode) -> void {
  mode = (inMode == Default) ? Default : static_cast<Mode>(mode ^ inMode);
}

auto HorizontalSliceToImageMapper::toggleHCL(bool active) -> void {
  isActive = active && !isActive;
}

auto HorizontalSliceToImageMapper::createImage(int iz) -> QImage {
  switch (mode) {
    case Data:
      if (isActive) {
        return mapSliceToImageHCL(iz);
      } else {
        return mapSliceToImage(iz);
      }
    case Default:
      return getImage();
    default:
      return {};
  }
}

auto HorizontalSliceToImageMapper::getWindComponent() -> QString {
  switch (component) {
    case 0:
      return "X";
    case 1:
      return "Y";
    case 2:
      return "Z";
    case 3:
      return "Betrag";
    default:
      return {};
  }
}

auto HorizontalSliceToImageMapper::getMode() -> Mode { return mode; }

auto HorizontalSliceToImageMapper::mapToRange(float x, float a, float b, int c,
                                              int d) -> float {
  return (x - a) / (b - a) * (float)((d - c) + c);
}
