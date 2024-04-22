#include "BGRImage.hpp"
#include "pixel.hpp"
#include "print.hpp"
#include "quadrant.hpp"
#include <array>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <string>
#include <vector>

static int WINDOW_SIZE;
static int QUADRANT_SIZE;
static int QUADRANT_AREA;

bool pixelInBounds(int x, int y, const cv::Size size) {
    return x >= 0 && x < size.height && y >= 0 && y < size.width;
}

void fillQuadrants(BGRPixel *quadrants[4], int counts[4], BGRPixel **image,
                   int x, int y, cv::Size size) {
    const int quadrantSize = ceil(WINDOW_SIZE / 2.0);
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
            auto pixelLuminosity =
                static_cast<uchar>(luminosity(neighbourPixel));
            BGRPixel bgrPixel = {static_cast<uchar>(neighbourPixel.data[0]),
                                 static_cast<uchar>(neighbourPixel.data[1]),
                                 static_cast<uchar>(neighbourPixel.data[2]),
                                 pixelLuminosity};
            // check which quadrants the pixel belongs to
            auto quadrantResult = checkQuadrant(i, j);

            // add the pixel to quadrant arrays
            quadrants[quadrantResult.q1][counts[quadrantResult.q1]++] =
                bgrPixel;
            // if the pixel belongs to two quadrants, add it to the second one
            if (quadrantResult.q2 != NONE) {
                quadrants[quadrantResult.q2][counts[quadrantResult.q2]++] =
                    bgrPixel;
            }
        }
    }
}

// calculate the standard deviation of a list of values
// operates only on the luminosity of the pixel
double standardDeviation(BGRPixel *values, int count) {
    double sum = 0.0;
    double variance = 0.0;
    for (int i = 0; i < count; i++) {
        sum += values[i].pixel.luminosity;
    }
    double mean = sum / count;
    for (int i = 0; i < count; i++) {
        variance += std::pow(values[i].pixel.luminosity - mean, 2);
    }
    return std::sqrt(variance / count);
}

double findIndexOfMinStdDev(BGRPixel *quadrants[4], int counts[4]) {
    int minIdx = -1;
    double minStdDev = 255;

    for (int i = 0; i < 4; i++) {
        if (counts[i] == 0) {
            continue;
        }
        double currStdDev = standardDeviation(quadrants[i], counts[i]);
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

BGRPixel avgOfQuadrant(BGRPixel *quadrant, int count) {
    BGRPixel avgPixel;
    for (int channel = 0; channel < 3; channel++) {
        double sum = 0;
        for (int i = 0; i < count; i++) {
            sum += quadrant[i].data[channel];
        }
        double average = sum / count;
        // set the pixel value to the average RGB of the quadrant
        avgPixel.data[channel] = static_cast<uchar>(average);
    }
    return avgPixel;
}

/// convert the image to black and white
void kuwahara(BGRPixel **image, BGRPixel **outputImage, cv::Size size,
              BGRPixel *quadrants[4], int quadrantCounts[4]) {
    // auto quadrants = std::array<std::vector<BGRPixel>, 4>();

    for (int x = 0; x < size.height; x++) {
        for (int y = 0; y < size.width; y++) {
            for (int i = 0; i < 4; i++) {
                quadrantCounts[i] = 0; // clear all quadrants
            }
            // loop through the entire window around this pixel
            // https://en.wikipedia.org/wiki/Kuwahara_filter#/media/File:Kuwahara.jpg
            fillQuadrants(quadrants, quadrantCounts, image, x, y, size);

            // after checking all quadrants, calculcate the standard deviations
            // check which quadrant has the minimum standard deviation
            int minIdx = findIndexOfMinStdDev(quadrants, quadrantCounts);

            // calculate the average of the BGR pixels in the minimum standard
            // deviation quadrant
            outputImage[x][y] = avgOfQuadrant(quadrants[minIdx], quadrantCounts[minIdx]);
        }
    }
}

// Entry point for the program
int main(int argc, char **argv) {
    auto inputPath = argv[1];
    auto outputPath = argv[2];
    if (argc < 3) {
        std::cerr << "Usage: kuwahara <input_image> <output_image> [--window window_size]" << std::endl;
        return -1;
    }

    if (argv[3] != nullptr && argv[4] != nullptr && std::string(argv[3]) == "--window") {
        WINDOW_SIZE = std::stoi(argv[4]);
        QUADRANT_SIZE = std::ceil((double)WINDOW_SIZE / 2.0);
        QUADRANT_AREA = (QUADRANT_SIZE * QUADRANT_SIZE) - 1;
    }

    cv::Mat bgrImage = cv::imread(inputPath, cv::IMREAD_COLOR);
    auto size = bgrImage.size();

    if (bgrImage.empty()) {
        std::cerr << "Error: Could not open or find the image" << std::endl;
        return -1;
    }

    BGRPixel **pixels = intoBGRPixelArray(bgrImage);
    BGRPixel **outputPixels = allocateBGRPixelArray(size);

    BGRPixel *quadrants[4] = {};
    for (int i = 0; i < 4; i++) {
        quadrants[i] = new BGRPixel[QUADRANT_AREA];
    }
    int quadrantCounts[4] = {0};

    // time the code execution
    auto start = std::chrono::high_resolution_clock::now();
    
    kuwahara(pixels, outputPixels, size, quadrants, quadrantCounts);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    print(elapsed.count());

    for (int i = 0; i < 4; i++) {
        delete[] quadrants[i];
    }

    auto outputMat = fromBGRPixelArray(outputPixels, size);
    cv::imwrite(outputPath, outputMat);

    deallocateBGRPixelArray(pixels, size);
    deallocateBGRPixelArray(outputPixels, size);

    return 0;
}
