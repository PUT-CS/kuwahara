#ifndef QUADRANT_H
#define QUADRANT_H

#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "pixel.cuh"
#include "print.hpp"

enum QuadrantKind { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, NONE = -1 };

struct QuadrantResult {
    QuadrantKind q1;
    QuadrantKind q2;
};

struct QuadrantData {
    // average BGR calculation
    unsigned bSum;
    unsigned gSum;
    unsigned rSum;
    size_t count;
    // Welford algorithm for calculating the variance
    // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
    double varianceMean;
    double varianceM2;
};

#endif // QUADRANT_H
