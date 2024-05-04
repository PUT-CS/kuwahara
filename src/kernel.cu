#include "pixel.cuh"
#include "quadrant.cuh"
#include "kernel.cuh"
#include "stdio.h"

__device__ void updateVariance(QuadrantData& quadrant, double newValue) {
    quadrant.count++;
    double delta = newValue - quadrant.varianceMean;
    quadrant.varianceMean += delta / quadrant.count;
    double delta2 = newValue - quadrant.varianceMean;
    quadrant.varianceM2 += delta * delta2;
}

__device__ double finalizeVariance(QuadrantData& quadrant) {
    if (quadrant.count < 2) {
        return 255; // return a large value to indicate that the variance is not valid
    }
    return quadrant.varianceM2 / quadrant.count; // sample variance
}

/// A pixel belongs to two quadrants at the same time.
/// Fills the 2 int indexes with the quadrant values, else -1.
/// The first field is always set. The second field is set only if the pixel belongs to two quadrants.
__device__ QuadrantResult checkQuadrant(int i, int j)
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
    return {NONE, NONE};
}

/// calculate the luminosity of a BGR pixel
__device__ double luminosity(const BGRPixel &pixel) {
    return 0.299 * pixel.pixel.r + 0.587 * pixel.pixel.g + 0.114 * pixel.pixel.b;
}

__device__ bool pixelInBounds(int x, int y, int sizeX, int sizeY) {
    return x >= 0 && x < sizeX && y >= 0 && y < sizeY;
}

__device__ void countPixel(QuadrantData &quadrantData, BGRPixel &pixel) {
    quadrantData.bSum += pixel.data[0];
    quadrantData.gSum += pixel.data[1];
    quadrantData.rSum += pixel.data[2];
    updateVariance(quadrantData, pixel.pixel.luminosity); // count is updated here
}

__device__ double findIndexOfMinStdDev(QuadrantData quadrants[4]) {
    int minIdx = -1;
    double minStdDev = 255;

    for (int i = 0; i < 4; i++) {
        if (quadrants[i].count == 0) {
            continue;
        }
        double currStdDev = std::sqrt(finalizeVariance(quadrants[i]));
        if (currStdDev < minStdDev) { // new minimum
            minStdDev = currStdDev;
            minIdx = i;
        }
    }
    return minIdx;
}

__device__ BGRPixel avgOfQuadrant(QuadrantData &quadrant) {
    return {static_cast<uchar>(quadrant.bSum / quadrant.count),
            static_cast<uchar>(quadrant.gSum / quadrant.count),
            static_cast<uchar>(quadrant.rSum / quadrant.count), 0};
}

__device__ void processQuadrants(QuadrantData quadrants[4], BGRPixel *image, int x, int y,
                                        int sizeX, int sizeY, int quadrantSize) {
    for (int i = -quadrantSize + 1; i < quadrantSize; i++) {
        for (int j = -quadrantSize + 1; j < quadrantSize; j++) {
            // check if the pixel is within the bounds of the image
            int pixelX = x + i, pixelY = y + j;
            if (!pixelInBounds(pixelX, pixelY, sizeX, sizeY) || i == 0 || j == 0)
                continue;
            auto &neighbourPixel = image[pixelY * sizeX + pixelX];

            // calculate luminosity of the rbg pixel to avod the problem
            // described here
            // https://en.wikipedia.org/wiki/Kuwahara_filter#Color_images
            auto pixelLuminosity = static_cast<uchar>(luminosity(neighbourPixel));
            BGRPixel bgrPixel = {static_cast<uchar>(neighbourPixel.data[0]),
                                 static_cast<uchar>(neighbourPixel.data[1]),
                                 static_cast<uchar>(neighbourPixel.data[2]), pixelLuminosity};

            // check which quadrants the pixel belongs to
            auto quadrantResult = checkQuadrant(i, j);
            
            // add the pixel to quadrant arrays
            countPixel(quadrants[quadrantResult.q1], bgrPixel);
            // if the pixel belongs to two quadrants, add it to the second one
            if (quadrantResult.q2 != NONE) {
                countPixel(quadrants[quadrantResult.q2], bgrPixel);
            }
        }
    }
}

__global__ void kuwahara(BGRPixel *image, BGRPixel *outputImage, int sizeX, int sizeY, int quadrantSize) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= sizeX || y >= sizeY) {
        return;
    }
    
    QuadrantData quadrants[4] = {};
    processQuadrants(quadrants, image, x, y, sizeX, sizeY, quadrantSize);
    int minIdx = findIndexOfMinStdDev(quadrants);
    outputImage[y * sizeX + x] = avgOfQuadrant(quadrants[minIdx]);
}

namespace KernelWrapper {
  void launchKuwaharaKernel(BGRPixel *image, BGRPixel *outputImage, int sizeX, int sizeY, int quadrantSize) {
    const int blockX = 16;
    const int blockY = 16;
    dim3 numberOfBlocks(sizeX / blockX + 1, sizeY / blockY + 1);
    dim3 numberOfThreads(blockX, blockY);
    kuwahara<<<numberOfBlocks, numberOfThreads>>>(image, outputImage, sizeX, sizeY, quadrantSize);
  }
}