#ifndef WELFORD_H
#define WELFORD_H

#include "quadrant.hpp"
#include <iostream>

inline void updateVariance(QuadrantData& quadrant, double newValue) {
    quadrant.count++;
    double delta = newValue - quadrant.varianceMean;
    quadrant.varianceMean += delta / quadrant.count;
    double delta2 = newValue - quadrant.varianceMean;
    quadrant.varianceM2 += delta * delta2;
}

inline double finalizeVariance(QuadrantData& quadrant) {
    if (quadrant.count < 2) {
        return 255; // return a large value to indicate that the variance is not valid
    }
    return quadrant.varianceM2 / quadrant.count; // sample variance
}

#endif // WELFORD_H
