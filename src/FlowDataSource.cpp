//
// Created by Joshua Lowe on 14.04.22.
//

#include "FlowDataSource.h"

#include <algorithm>
#include <cmath>
#include <limits>

FlowDataSource::FlowDataSource() : dimension(16), frame(0){};

FlowDataSource::FlowDataSource(int dimension)
    : dimension(dimension), frame(0){};

auto FlowDataSource::getArray() -> QVector<QVector3D> {
  return cartesianDataGrid;
};

auto FlowDataSource::createData() -> void {
  cartesianDataGrid.clear();
  genTornado(frame);
}

auto FlowDataSource::setFrame(int inFrame) -> void { frame = inFrame; }

auto FlowDataSource::getFrame() -> int { return frame; }

auto FlowDataSource::setDimension(int resolution) -> void {
  dimension =
      (dimension + resolution <= 0) ? dimension : dimension + resolution;
}

auto FlowDataSource::getDataValue(int ix, int iy, int iz, int ic) -> float {
  int index = ix + (dimension * iy) + (dimension * dimension * iz);
  switch (ic) {
    case 0:
      return cartesianDataGrid.at(index).x();
      break;
    case 1:
      return cartesianDataGrid.at(index).y();
      break;
    case 2:
      return cartesianDataGrid.at(index).z();
      break;
    case 3:
      return getBetrag(ix, iy, iz);
      break;
    default:
      return {};
  }
}

auto FlowDataSource::printValuesOfHorizontalSlice(int iz) -> void {
  float xc, yc, zc;
  for (int i = 0; i < dimension; i++) {
    for (int j = 0; j < dimension; j++) {
      xc = getDataValue(j, i, iz, 0);
      yc = getDataValue(j, i, iz, 1);
      zc = getDataValue(j, i, iz, 2);
      printf("(%d %d):(%f, %f, %f), ", j, i, xc, yc, zc);
    }
    printf("\n");
  }
}

auto FlowDataSource::getMinBetrag() -> float {
  float minimum = std::numeric_limits<float>::max();
  for (int i = 0; i < dimension; i++) {
    for (int j = 0; j < dimension; j++) {
      for (int u = 0; u < dimension; u++) {
        float temp = getBetrag(u, j, i);
        minimum = (temp < minimum) ? temp : minimum;
      }
    }
  }
  return minimum;
}

auto FlowDataSource::getMaxBetrag() -> float {
  float maximum = 0;
  for (int i = 0; i < dimension; i++) {
    for (int j = 0; j < dimension; j++) {
      for (int u = 0; u < dimension; u++) {
        float temp = getBetrag(u, j, i);
        maximum = (temp > maximum) ? temp : maximum;
      }
    }
  }
  return maximum;
}

auto FlowDataSource::getMaxValue(int c) -> float {
  float maximum = std::numeric_limits<float>::lowest();
  for (int i = 0; i < dimension; i++) {
    for (int j = 0; j < dimension; j++) {
      for (int u = 0; u < dimension; u++) {
        float temp = getDataValue(j, i, u, 0);
        maximum = (temp > maximum) ? temp : maximum;
        temp = getDataValue(j, i, u, 1);
        maximum = (temp > maximum) ? temp : maximum;
        temp = getDataValue(j, i, u, 2);
        maximum = (temp > maximum) ? temp : maximum;
      }
    }
  }
  return maximum;
}
auto FlowDataSource::getMinValue(int c) -> float {
  float minimum = std::numeric_limits<float>::max();
  for (int i = 0; i < dimension; i++) {
    for (int j = 0; j < dimension; j++) {
      for (int u = 0; u < dimension; u++) {
        float temp = getDataValue(j, i, u, 0);
        minimum = (temp < minimum) ? temp : minimum;
        temp = getDataValue(j, i, u, 1);
        minimum = (temp < minimum) ? temp : minimum;
        temp = getDataValue(j, i, u, 2);
        minimum = (temp < minimum) ? temp : minimum;
      }
    }
  }
  return minimum;
}

auto FlowDataSource::getBetrag(int x, int y, int z) -> float {
  float betrag;
  float xc2 = std::powf(getDataValue(x, y, z, 0), 2);
  float yc2 = std::powf(getDataValue(x, y, z, 1), 2);
  float zc2 = std::powf(getDataValue(x, y, z, 2), 2);
  betrag = std::sqrtf(xc2 + yc2 + zc2);
  return betrag;
}

auto FlowDataSource::getDimension() -> int { return dimension; }

auto FlowDataSource::reverseArray() -> void {
  std::reverse(cartesianDataGrid.begin(), cartesianDataGrid.end());
}

auto FlowDataSource::genTornado(int time) -> void {
  /*
   *  Gen_Tornado creates a vector field of dimension [xs,ys,zs,3] from
   *  a proceedural function. By passing in different time arguements,
   *  a slightly different and rotating field is created.
   *
   *  The magnitude of the vector field is highest at some funnel shape
   *  and values range from 0.0 to around 0.4 (I think).
   *
   *  I just wrote these comments, 8 years after I wrote the function.
   *
   * Developed by Roger A. Crawfis, The Ohio State University
   *
   */
  int xs = dimension, ys = dimension, zs = dimension;
  float x, y, z;
  int ix, iy, iz;
  float r, xc, yc, scale, temp, z0;
  float r2 = 8;
  float SMALL = 0.00000000001;
  float xdelta = 1.0 / (xs - 1.0);
  float ydelta = 1.0 / (ys - 1.0);
  float zdelta = 1.0 / (zs - 1.0);

  for (iz = 0; iz < zs; iz++) {
    z = iz * zdelta;  // map z to 0->1
    xc = 0.5 +
         0.1 * sin(0.04 * time +
                   10.0 * z);  // For each z-slice, determine the spiral circle.
    yc = 0.5 +
         0.1 * cos(0.03 * time +
                   3.0 * z);  //    (xc,yc) determine the center of the circle.
    r = 0.1 + 0.4 * z * z +
        0.1 * z * sin(8.0 * z);  //  The radius also changes at each z-slice.
    r2 = 0.2 + 0.1 * z;          //    r is the center radius, r2 is for damping
    for (iy = 0; iy < ys; iy++) {
      y = iy * ydelta;
      for (ix = 0; ix < xs; ix++) {
        x = ix * xdelta;
        temp = sqrt((y - yc) * (y - yc) + (x - xc) * (x - xc));
        scale = fabs(r - temp);
        /*
         *  I do not like this next line. It produces a discontinuity
         *  in the magnitude. Fix it later.
         *
         */
        if (scale > r2)
          scale = 0.8 - scale;
        else
          scale = 1.0;
        z0 = 0.1 * (0.1 - temp * z);
        if (z0 < 0.0) z0 = 0.0;
        temp = sqrt(temp * temp + z0 * z0);
        scale = (r + r2 - temp) * scale / (temp + SMALL);
        scale = scale / (1 + z);
        float xResult, yResult, zResult;
        xResult = scale * (y - yc) + 0.1 * (x - xc);
        yResult = scale * -(x - xc) + 0.1 * (y - yc);
        zResult = scale * z0;
        cartesianDataGrid.append({xResult, yResult, zResult});
      }
    }
  }
}
