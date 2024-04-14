#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <list>
#include <opencv2/core/cvdef.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <string>

// generic print function
template <typename T>
void print(T t) { std::cout << t << std::endl; }

// print function for multiple arguments
template <typename T, typename... Args>
void print(T t, Args... args)
{
    std::cout << t << " ";
    print(args...);
}

typedef cv::Vec3b Pixel;

constexpr const double WINDOW_SIZE = 19.0;

bool pixelInBounds(const cv::Mat& image, int x, int y)
{
    return x >= 0 && x < image.size().width && y >= 0 && y < image.size().height;
}

enum Quadrant { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, NONE = -1 };

struct QuadrantResult {
    Quadrant q1;
    Quadrant q2;
};

// print the enum as string
std::ostream& operator<<(std::ostream& os, const Quadrant& quadrant)
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
QuadrantResult checkQuadrant(int i, int j)
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
    return { NONE, NONE };
}

// print quadrant result
std::ostream& operator<<(std::ostream& os, const QuadrantResult& result)
{
    os << "Quadrant 1: " << result.q1 << ", Quadrant 2: " << result.q2;
    return os;
}

typedef uchar PixelValue;

typedef union {
    struct BGRPixel {
        uchar b;
        uchar g;
        uchar r;
        uchar luminosity;
    } pixel;
    uchar data[4];
} BGRPixel;

// calculate the standard deviation of a list of values
// operates only on the luminosity of the pixel
double standardDeviation(const std::vector<BGRPixel>& values)
{
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

/// calculate the luminosity of a BGR pixel
double luminosity(const Pixel& pixel)
{
    return 0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0];
}

/// convert the image to black and white
void kuwahara(cv::Mat& image, cv::Mat& outputImage)
{
    const int height = image.size().height;
    const int width = image.size().width;
    
    const int quadrantSize = ceil(WINDOW_SIZE / 2.0);
    auto quadrants = std::array<std::vector<BGRPixel>, 4>();

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            auto& pixel = image.at<Pixel>(x, y);
            // loop through the entire window around this pixel
            // https://en.wikipedia.org/wiki/Kuwahara_filter#/media/File:Kuwahara.jpg

            // clear all quadrants
            for (auto& quadrant : quadrants) {
                quadrant.clear();
            }

            for (int i = -quadrantSize + 1; i < quadrantSize; i++) {
                for (int j = -quadrantSize + 1; j < quadrantSize; j++) {
                    // check if the pixel is within the bounds of the image
                    int pixelX = x + i, pixelY = y + j;
                    if (!pixelInBounds(image, pixelX, pixelY)) {
                        continue;
                    }
                    if (i == 0 && j == 0) {
                        continue;
                    }

                    auto& neighbourPixel = image.at<Pixel>(pixelX, pixelY);

                    // calculate luminosity of the rbg pixel to avod the problem described in
                    // https://en.wikipedia.org/wiki/Kuwahara_filter#Color_images
                    auto pixelLuminosity = static_cast<uchar>(luminosity(neighbourPixel));
                    BGRPixel bgrPixel = { neighbourPixel[0], neighbourPixel[1], neighbourPixel[2], pixelLuminosity };

                    // check which quadrants the pixel belongs to
                    auto quadrantResult = checkQuadrant(i, j);

                    // add the pixel to our lists
                    quadrants[quadrantResult.q1].push_back(bgrPixel);

                    // if the pixel belongs to two quadrants, add it to the second quadrant
                    if (quadrantResult.q2 != NONE) {
                        quadrants[quadrantResult.q2].push_back(bgrPixel);
                    }
                }
                
                // after checking all quadrants, calculcate the standard deviations
                int minIdx = 0;
                double minStdDev = 256.0;
                for (int i = 0; i < 4; i++) {
                    double currStdDev = standardDeviation(quadrants[i]);
                    if (currStdDev < minStdDev) { // new minimum
                        minStdDev = currStdDev;
                        minIdx = i;
                    }
                }

                // calculate the average of the BGR pixels in the minimum standard deviation quadrant
                for (int channel = 0; channel < 3; channel++) {
                    double sum = 0;
                    for (const auto &value : quadrants[minIdx]) {
                        sum += value.data[channel];
                    }
                    double average = sum / quadrants[minIdx].size();
                    // set the pixel value to the average RGB of the quadrant
                    outputImage.at<Pixel>(x, y)[channel] = static_cast<uchar>(average);
                }
            }
        }
    }
}

// Entry point for the program
int main(int argc, char** argv)
{
    auto inputPath = argv[1];
    auto outputPath = argv[2];

    cv::Mat bgrImage = cv::imread(inputPath, cv::IMREAD_COLOR);

    if (bgrImage.empty()) {
        std::cerr << "Error: Could not open or find the image" << std::endl;
        return -1;
    }

    auto outputImage = bgrImage.clone();

    kuwahara(bgrImage, outputImage);
    print("Done");

    cv::imwrite(outputPath, outputImage);

    return 0;
}
