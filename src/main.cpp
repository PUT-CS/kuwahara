#include "pixel.hpp"
#include "quadrant.hpp"
#include <array>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <string>

constexpr const double WINDOW_SIZE = 9.0;

bool pixelInBounds(const cv::Mat &image, int x, int y) {
    return x >= 0 && x < image.size().height && y >= 0 &&
           y < image.size().width;
}

/// convert the image to black and white
void kuwahara(cv::Mat &image, cv::Mat &outputImage) {
    const int quadrantSize = ceil(WINDOW_SIZE / 2.0);
    auto quadrants = std::array<std::vector<BGRPixel>, 4>();
    quadrants.fill(std::vector<BGRPixel>());

    for (int x = 0; x < image.size().height; x++) {
        for (int y = 0; y < image.size().width; y++) {
            auto &pixel = image.at<Pixel>(x, y);

            // loop through the entire window around this pixel
            // https://en.wikipedia.org/wiki/Kuwahara_filter#/media/File:Kuwahara.jpg

            // clear all quadrants
            for (auto &quadrant : quadrants) {
                quadrant.clear();
            }

            // print("Checking quadrants for pixel at ", x, y);
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
                    auto &neighbourPixel = image.at<Pixel>(pixelX, pixelY);

                    // calculate luminosity of the rbg pixel to avod the problem
                    // described in
                    // https://en.wikipedia.org/wiki/Kuwahara_filter#Color_images
                    auto pixelLuminosity =
                        static_cast<uchar>(luminosity(neighbourPixel));
                    BGRPixel bgrPixel = {static_cast<uchar>(neighbourPixel[0]),
                                         static_cast<uchar>(neighbourPixel[1]),
                                         static_cast<uchar>(neighbourPixel[2]),
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

            // after checking all quadrants, calculcate the standard deviations
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

            // calculate the average of the BGR pixels in the minimum standard
            // deviation quadrant
            auto &outputPixel = outputImage.at<Pixel>(x, y);

            for (int channel = 0; channel < 3; channel++) {
                double sum = 0;
                for (const auto &value : quadrants[minIdx]) {
                    sum += value.data[channel];
                }
                double average = sum / quadrants[minIdx].size();

                // set the pixel value to the average RGB of the quadrant
                outputPixel[channel] = static_cast<uchar>(average);
            }
        }
    }
}

// Entry point for the program
int main(int argc, char **argv) {
    auto inputPath = argv[1];
    auto outputPath = argv[2];

    cv::Mat bgrImage = cv::imread(inputPath, cv::IMREAD_COLOR);

    if (bgrImage.empty()) {
        std::cerr << "Error: Could not open or find the image" << std::endl;
        return -1;
    }

    auto outputImage = cv::Mat(bgrImage.size(), bgrImage.type());

    kuwahara(bgrImage, outputImage);

    cv::imwrite(outputPath, outputImage);

    return 0;
}
