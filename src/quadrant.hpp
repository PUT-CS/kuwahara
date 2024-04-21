#ifndef QUADRANT_H
#define QUADRANT_H

#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include "pixel.hpp"
#include "print.hpp"

enum Quadrant {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    NONE = -1
};

struct QuadrantResult {
    Quadrant q1;
    Quadrant q2;
};

typedef std::array<std::vector<BGRPixel>, 4> Quadrants;

// print the enum as string
inline std::ostream& operator<<(std::ostream& os, const Quadrant& quadrant)
{
    switch (quadrant) {
    case Quadrant::TOP_LEFT:
        os << "TOP_LEFT";
        break;
    case Quadrant::TOP_RIGHT:
        os << "TOP_RIGHT";
        break;
    case Quadrant::BOTTOM_LEFT:
        os << "BOTTOM_LEFT";
        break;
    case Quadrant::BOTTOM_RIGHT:
        os << "BOTTOM_RIGHT";
        break;
    default:
        os << "";
        break;
    }

    return os;
}

/// A pixel belongs to two quadrants at the same time.
/// Fills the 2 int indexes with the quadrant values, else -1.
/// The first field is always set. The second field is set only if the pixel belongs to two quadrants.
inline QuadrantResult checkQuadrant(int i, int j)
{
    // base cases where the pixel belongs to just one quadrant
    if (i < 0 && j < 0) {
        return { TOP_LEFT, NONE };
    } else if (i > 0 && j < 0) {
        return { TOP_RIGHT, NONE };
    } else if (i < 0 && j > 0) {
        return { BOTTOM_LEFT, NONE };
    } else if (i > 0 && j > 0) {
        return { BOTTOM_RIGHT, NONE };
    }

    // pixels that belong to two quadrants at once
    if (i == 0 && j < 0) {
        return { TOP_LEFT, TOP_RIGHT };
    } else if (i == 0 && j > 0) {
        return { BOTTOM_LEFT, BOTTOM_RIGHT };
    } else if (i < 0 && j == 0) {
        return { TOP_LEFT, BOTTOM_LEFT };
    } else if (i > 0 && j == 0) {
        return { TOP_RIGHT, BOTTOM_RIGHT };
    }

    // should never happen, we ignore the central pixel
    print("Error: Pixel belongs to no quadrant");
    return { NONE, NONE };
}

// print quadrant result
inline std::ostream& operator<<(std::ostream& os, const QuadrantResult& result)
{
    os << "Quadrant 1: " << result.q1 << ", Quadrant 2: " << result.q2;
    return os;
}

// calculate the standard deviation of a list of values
// operates only on the luminosity of the pixel
inline double standardDeviation(const std::vector<BGRPixel> &values) {
  double sum = 0.0;
  double variance = 0.0;

  for (int i = 0; i < values.size(); i++) {
    sum += values[i].pixel.luminosity;
  }

  double mean = sum / values.size();
  for (int i = 0; i < values.size(); i++) {
    variance += std::pow(values[i].pixel.luminosity - mean, 2);
  }

  return std::sqrt(variance / values.size());
}

#endif // QUADRANT_H
