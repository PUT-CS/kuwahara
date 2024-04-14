#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <list>
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

constexpr const double WINDOW_SIZE = 9.0;

constexpr int32_t myCeil(float num)
{
    return (static_cast<float>(static_cast<int32_t>(num)) == num)
        ? static_cast<int32_t>(num)
        : static_cast<int32_t>(num) + ((num > 0) ? 1 : 0);
}

bool pixelInBounds(const cv::Mat& image, int x, int y)
{
    return x >= 0 && x < image.size().width && y >= 0 && y < image.size().height;
}

enum Quadrant { TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    NONE = -1 };
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

// calculate the standard deviation of a list of values
double standardDeviation(const std::vector<int>& values)
{
    double sum = 0.0;
    double variance = 0.0;

    for (int i = 0; i < values.size(); i++) {
        sum += values[i];
    }

    double mean = sum / values.size();
    for (int i = 0; i < values.size(); i++) {
        variance += std::pow(values[i] - mean, 2);
    }

    return std::sqrt(variance / values.size());
}

typedef int PixelValue;

/// calculate the luminosity of a BGR pixel
double luminosity(const Pixel& pixel)
{
    return 0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0];
}

struct QuadrantEntry {
    int luminosity;
    uchar b;
    uchar g;
    uchar r;
};

/// convert the image to black and white
void kuwahara(cv::Mat& image, cv::Mat& outputImage)
{
    constexpr int quadrantSize = myCeil(WINDOW_SIZE / 2.0);
    constexpr int quadrantArea = quadrantSize * quadrantSize - 1; // ignore the central pixel
    auto quadrants = std::array<std::vector<PixelValue>, 4>();
    const int height = image.size().height;
    const int width = image.size().width;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            auto& pixel = image.at<Pixel>(x, y);
            // const int value = static_cast<int>(pixel[2]);
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

                    Pixel neighbourPixel = image.at<Pixel>(pixelX, pixelY);
                    auto neighbourLuminosity = static_cast<int>(luminosity(neighbourPixel));

                    auto quadrantResult = checkQuadrant(i, j);
                    // add the pixel to our lists
                    quadrants[quadrantResult.q1].push_back(neighbourLuminosity);

                    // if the pixel belongs to two quadrants, add it to the second quadrant
                    if (quadrantResult.q2 != NONE) {
                        quadrants[quadrantResult.q2].push_back(neighbourLuminosity);
                    }
                }
                // after checking all quadrants, calculcate the standard deviations
                // index of minimum standard deviation
                int minIdx = 0;
                double minStdDev = 255.0;
                for (int i = 0; i < 4; i++) {
                    double currStdDev = standardDeviation(quadrants[i]);
                    if (currStdDev < minStdDev) {
                        minStdDev = currStdDev;
                        minIdx = i;
                    }
                }

                // calculate the average of the RBG pixels in the minimum standard deviation quadrant
                double sum = 0.0;
                for (const auto& value : quadrants[minIdx]) {
                    sum += value;
                }
                double average = sum / quadrants[minIdx].size();

                // set the pixel value to the average RGB of the quadrant
                outputImage.at<Pixel>(x, y)[0] = static_cast<uchar>(average);
                outputImage.at<Pixel>(x, y)[1] = static_cast<uchar>(average);
                outputImage.at<Pixel>(x, y)[2] = static_cast<uchar>(average);
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

// int main(int argc, char** argv)
// {
//     // Load the BGR image
//     cv::Mat bgrImage = cv::imread(argv[1], cv::IMREAD_COLOR);

//     if (bgrImage.empty()) {
//         std::cerr << "Could not open or find the image" << std::endl;
//         return -1;
//     }

//     // Convert BGR image to HSV
//     cv::Mat hsvImage;
//     cvtColor(bgrImage, hsvImage, cv::COLOR_BGR2HSV);

//     // Access pixel values at a specific position (100, 100)
//     int y = 100;
//     int x = 100;

//     // Check if the coordinates are within the image bounds
//     if (x >= 0 && x < hsvImage.cols && y >= 0 && y < hsvImage.rows) {
//         // Access pixel values
//         cv::Vec3b pixel = hsvImage.at<cv::Vec3b>(y, x);
//         uchar hue = pixel[0]; // Hue value
//         uchar saturation = pixel[1]; // Saturation value
//         uchar value = pixel[2]; // Value (brightness) value

//         // Convert saturation and value to percentages
//         float saturationPercentage = static_cast<float>(saturation) / 255.0f * 100.0f;
//         float valuePercentage = static_cast<float>(value) / 255.0f * 100.0f;

//         // Print the HSV values in proper format
//         std::cout << "HSV values at position (" << x << ", " << y << "): ";
//         std::cout << "(" << static_cast<int>(hue * 2) << ", " << saturationPercentage << "%, " << valuePercentage << "%)" << std::endl;
//     } else {
//         std::cerr << "Coordinates out of bounds" << std::endl;
//         return -1;
//     }

//     return 0;
// }
