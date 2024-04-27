#include "BGRImage.hpp"
#include "pixel.hpp"
#include "print.hpp"
#include "quadrant.hpp"
#include "welford.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <string>
#include <vector>

inline bool pixelInBounds(int x, int y, const cv::Size size) {
    return x >= 0 && x < size.height && y >= 0 && y < size.width;
}

inline void countPixel(QuadrantData &quadrantData, BGRPixel &pixel) {
    quadrantData.bSum += pixel.data[0];
    quadrantData.gSum += pixel.data[1];
    quadrantData.rSum += pixel.data[2];
    updateVariance(quadrantData, pixel.pixel.luminosity); // count is updated here
}

inline double findIndexOfMinStdDev(QuadrantData quadrants[4]) {
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
    if (minIdx == -1) {
        print("Error: No minimum standard deviation found");
        exit(1);
    }
    return minIdx;
}

inline BGRPixel avgOfQuadrant(QuadrantData &quadrant) {
    return {static_cast<uchar>(quadrant.bSum / quadrant.count),
            static_cast<uchar>(quadrant.gSum / quadrant.count),
            static_cast<uchar>(quadrant.rSum / quadrant.count), 0};
}

inline void processQuadrants(QuadrantData quadrants[4], BGRPixel **image, int x, int y,
                             cv::Size size, int quadrantSize) {
    for (int i = -quadrantSize + 1; i < quadrantSize; i++) {
        for (int j = -quadrantSize + 1; j < quadrantSize; j++) {
            // check if the pixel is within the bounds of the image
            int pixelX = x + i, pixelY = y + j;
            if (!pixelInBounds(pixelX, pixelY, size) || i == 0 || j == 0)
                continue;
            auto &neighbourPixel = image[pixelX][pixelY];

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

void kuwahara(BGRPixel **image, BGRPixel **outputImage, cv::Size size, int quadrantSize) {
    for (int x = 0; x < size.height; x++) {
        for (int y = 0; y < size.width; y++) {
            QuadrantData quadrants[4];
            for (int i = 0; i < 4; i++) {
                quadrants[i] = {0, 0, 0, 0, 0, 0};
            }
            // loop through the entire window around this pixel
            // https://en.wikipedia.org/wiki/Kuwahara_filter#/media/File:Kuwahara.jpg
            processQuadrants(quadrants, image, x, y, size, quadrantSize);

            // after checking all quadrants, calculcate the standard deviations
            // check which quadrant has the minimum standard deviation
            int minIdx = findIndexOfMinStdDev(quadrants);

            // calculate the average of the BGR pixels in the min stddev quadrant
            outputImage[x][y] = avgOfQuadrant(quadrants[minIdx]);
        }
    }
}

// Entry point for the program
int main(int argc, char **argv) {
    auto inputPath = argv[1];
    auto outputPath = argv[2];
    if (argc < 3) {
        std::cerr << "Usage: kuwahara <input_image> <output_image> [--window window_size]"
                  << std::endl;
        return -1;
    }

    int windowSize = 9;
    if (argv[3] != nullptr && argv[4] != nullptr && std::string(argv[3]) == "--window") {
        windowSize = std::stoi(argv[4]);
        if (windowSize % 2 == 0) {
            std::cerr << "Error: Window size must be an odd number" << std::endl;
            return -1;
        }
    }

    cv::Mat bgrImage = cv::imread(inputPath, cv::IMREAD_COLOR);
    auto size = bgrImage.size();

    if (bgrImage.empty()) {
        std::cerr << "Error: Could not open or find the image" << std::endl;
        return -1;
    }

    BGRPixel **pixels = intoBGRPixelArray(bgrImage);
    BGRPixel **outputPixels = allocateBGRPixelArray(size);

    auto start = std::chrono::high_resolution_clock::now();
    int quadrantSize = std::ceil(windowSize / 2.0);
    
    kuwahara(pixels, outputPixels, size, quadrantSize);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    print(elapsed.count());

    auto outputMat = fromBGRPixelArray(outputPixels, size);
    cv::imwrite(outputPath, outputMat);

    freeBGRPixelArray(pixels, size);
    freeBGRPixelArray(outputPixels, size);

    return 0;
}
