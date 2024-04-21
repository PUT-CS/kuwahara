#include "pixel.hpp"
#include "print.hpp"
#include "quadrant.hpp"
#include "BGRImage.hpp"
#include <array>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <string>
#include <vector>

constexpr const double WINDOW_SIZE = 9.0;

bool pixelInBounds(int x, int y, const cv::Size size) {
    return x >= 0 && x < size.height && y >= 0 && y < size.width;
}

void fillQuadrants(Quadrants& quadrants, BGRPixel** image, int x, int y, cv::Size size) {
    const int quadrantSize = ceil(WINDOW_SIZE / 2.0);
    for (int i = -quadrantSize + 1; i < quadrantSize; i++) {
        for (int j = -quadrantSize + 1; j < quadrantSize; j++) {
            // check if the pixel is within the bounds of the image
            int pixelX = x + i, pixelY = y + j;
            if (!pixelInBounds(pixelX, pixelY, size)) {
                continue;
            }
            if (i == 0 && j == 0) {
                continue;
            }
            auto &neighbourPixel = image[pixelX][pixelY];

            // calculate luminosity of the rbg pixel to avod the problem
            // described in
            // https://en.wikipedia.org/wiki/Kuwahara_filter#Color_images
            auto pixelLuminosity =
                static_cast<uchar>(luminosity(neighbourPixel));
            BGRPixel bgrPixel = {static_cast<uchar>(neighbourPixel.data[0]),
                                 static_cast<uchar>(neighbourPixel.data[1]),
                                 static_cast<uchar>(neighbourPixel.data[2]),
                                 pixelLuminosity};
            // check which quadrants the pixel belongs to
            auto quadrantResult = checkQuadrant(i, j);

            // add the pixel to our lists
            quadrants[quadrantResult.q1].push_back(bgrPixel);
            // if the pixel belongs to two quadrants, add it to the
            // second quadrant
            if (quadrantResult.q2 != NONE) {
                quadrants[quadrantResult.q2].push_back(bgrPixel);
            }
        }
    }
}

double findIndexOfMinStdDev(const Quadrants& quadrants) {
    int minIdx = -1;
    double minStdDev = 255;

    for (auto &quadrant : quadrants) {
        if (quadrant.empty()) {
            continue;
        }
        double currStdDev = standardDeviation(quadrant);
        if (currStdDev < minStdDev) { // new minimum
            minStdDev = currStdDev;
            minIdx = &quadrant - &quadrants[0];
        }
    }
    if (minIdx == -1) {
        print("Error: No minimum standard deviation found");
        exit(1);
    }
    return minIdx;
}

// BGRPixel avgOfQuadrant(const std::vector<BGRPixel>& quadrant) {
//     BGRPixel avgPixel = {0, 0, 0, 0};
//     for (const auto &pixel : quadrant) {
//         double sum = 0;
//         for (int i = 0; i < 3; i++) {
//             sum += pixel.data[i];
//         }
//         avgPixel.data[0] = static_cast<uchar>(sum / quadrant.size());
//     }
//     return avgPixel;
// }

/// convert the image to black and white
void kuwahara(BGRPixel** image, BGRPixel** outputImage, cv::Size size) {
    
    auto quadrants = std::array<std::vector<BGRPixel>, 4>();
    quadrants.fill(std::vector<BGRPixel>());

    for (int x = 0; x < size.height; x++) {
        for (int y = 0; y < size.width; y++) {
            auto &pixel = image[x][y];

            // loop through the entire window around this pixel
            // https://en.wikipedia.org/wiki/Kuwahara_filter#/media/File:Kuwahara.jpg

            // clear all quadrants
            for (auto &quadrant : quadrants) {
                quadrant.clear();
            }

            fillQuadrants(quadrants, image, x, y, size);

            // after checking all quadrants, calculcate the standard deviations
            auto minIdx = findIndexOfMinStdDev(quadrants);

            // calculate the average of the BGR pixels in the minimum standard
            // deviation quadrant
            auto &outputPixel = outputImage[x][y];

            // auto avgPixel = avgOfQuadrant(quadrants[minIdx]);

            for (int channel = 0; channel < 3; channel++) {
                double sum = 0;
                for (const auto &value : quadrants[minIdx]) {
                    sum += value.data[channel];
                }
                double average = sum / quadrants[minIdx].size();
                
                // set the pixel value to the average RGB of the quadrant
                outputPixel.data[channel] = static_cast<uchar>(average);
                //outputPixel.data[channel] = avgPixel.data[channel];
            }
        }
    }
}

// Entry point for the program
int main(int argc, char **argv) {
    auto inputPath = argv[1];
    auto outputPath = argv[2];

    cv::Mat bgrImage = cv::imread(inputPath, cv::IMREAD_COLOR);
    auto size = bgrImage.size();

    if (bgrImage.empty()) {
        std::cerr << "Error: Could not open or find the image" << std::endl;
        return -1;
    }

    BGRPixel** pixels = intoBGRPixelArray(bgrImage);
    BGRPixel** outputPixels = allocateBGRPixelArray(size);

    kuwahara(pixels, outputPixels, size);

    auto outputMat = fromBGRPixelArray(outputPixels, size);
    
    cv::imwrite(outputPath, outputMat);
    deallocateBGRPixelArray(pixels, size);
    deallocateBGRPixelArray(outputPixels, size);

    return 0;
}
