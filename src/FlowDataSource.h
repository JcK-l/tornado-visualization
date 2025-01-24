//
// Created by Joshua Lowe on 14.04.22.
//

#ifndef CODE_FLOWDATASOURCE_H
#define CODE_FLOWDATASOURCE_H

#include <QtGui/qvector3d.h>

#include <QVector3D>
#include <QVector>
#include <tuple>
#include <vector>

class FlowDataSource {
 public:
  FlowDataSource();
  explicit FlowDataSource(int);

  auto printValuesOfHorizontalSlice(int) -> void;
  auto createData() -> void;
  auto setDimension(int) -> void;

  auto getDataValue(int, int, int, int) -> float;
  auto getArray() -> QVector<QVector3D>;
  auto getBetrag(int, int, int) -> float;
  auto getMaxValue(int) -> float;
  auto getMinValue(int) -> float;
  auto getMinBetrag() -> float;
  auto getMaxBetrag() -> float;
  auto getDimension() -> int;
  auto reverseArray() -> void;
  auto setFrame(int) -> void;
  auto getFrame() -> int;

 private:
  auto genTornado(int) -> void;
  QVector<QVector3D> cartesianDataGrid;

  int dimension;
  int frame;
};

#endif  // CODE_FLOWDATASOURCE_H
