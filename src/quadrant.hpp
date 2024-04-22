#ifndef QUADRANT_H
#define QUADRANT_H

#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include "pixel.hpp"
#include "print.hpp"

enum QuadrantKind { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, NONE = -1 };

struct QuadrantContainer {
    BGRPixel* quadrants[4]; // 4 bgr pixel arrays
    int write_counts[4]; // 4 counting arrays
};

inline void allocateQuadrants(QuadrantContainer &container, int quadrantArea) {
    for (int i = 0; i < 4; i++) {
        container.quadrants[i] = new BGRPixel[quadrantArea]();
        container.write_counts[i] = 0;
    }
}

inline void pushToQuadrant(QuadrantContainer &container, int i, BGRPixel pixel) {
    container.quadrants[i][container.write_counts[i]++] = pixel;
}

inline void cleanQuadrants(QuadrantContainer &container) {
    for (int i = 0; i < 4; i++) {
        container.write_counts[i] = 0;
    }
}

inline void deallocateQuadrants(QuadrantContainer &container) {
    for (int i = 0; i < 4; i++) {
        delete[] container.quadrants[i];
    }
}

struct QuadrantResult {
    QuadrantKind q1;
    QuadrantKind q2;
};

typedef std::array<std::vector<BGRPixel>, 4> Quadrants;

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

#endif // QUADRANT_H
