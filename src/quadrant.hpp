#ifndef QUADRANT_H
#define QUADRANT_H

#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "pixel.hpp"
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

/// A pixel belongs to two quadrants at the same time.
/// Fills the 2 int indexes with the quadrant values, else -1.
/// The first field is always set. The second field is set only if the pixel belongs to two quadrants.
inline QuadrantResult checkQuadrant(int i, int j)
{
    if (i < 0 && j < 0) {
        return { TOP_LEFT, NONE };
    } else if (i > 0 && j < 0) {
        return { TOP_RIGHT, NONE };
    } else if (i < 0 && j > 0) {
        return { BOTTOM_LEFT, NONE };
    } else if (i > 0 && j > 0) {
        return { BOTTOM_RIGHT, NONE };
    } else if (i == 0) {
        if (j < 0) {
            return { TOP_LEFT, TOP_RIGHT };
        } else if (j > 0) {
            return { BOTTOM_LEFT, BOTTOM_RIGHT };
        }
    } else if (j == 0) {
        if (i < 0) {
            return { TOP_LEFT, BOTTOM_LEFT };
        } else if (i > 0) {
            return { TOP_RIGHT, BOTTOM_RIGHT };
        }
    }
    
    throw std::runtime_error("Pixel doesn't belong to any quadrant (logic error)");
}

#endif // QUADRANT_H
