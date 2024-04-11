#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <algorithm>

// generic print function
template <typename T> void print(T t) { std::cout << t << std::endl; }

// print function for multiple arguments
template <typename T, typename... Args> void print(T t, Args... args) {
    std::cout << t << " ";
    print(args...);
}

struct BGRColor {
    uint8_t b;
    uint8_t g;
    uint8_t r;
};

constexpr BGRColor WHITE = {255, 255, 255};

void setPixel(cv::Mat& image, int x, int y, BGRColor color) {
    auto& pixel = image.at<cv::Vec3i>(x, y);
    pixel[0] = color.b;
    pixel[1] = color.g;
    pixel[2] = color.r;
}

void corruptImage(cv::Mat& image, cv::Mat& outputImage, int percentage) {
    for (int x = 0; x < image.size().width; x++) {
        for (int y = 0; y < image.size().height; y++) {
            if (rand() % 100 < percentage) {
                setPixel(outputImage, x, y, WHITE);
            }
        }
    }
}

constexpr int RADIUS_X = 4;
constexpr int RADIUS_Y = 4;

// Function to apply the median filter to the image
void medianFilter(cv::Mat& image, cv::Mat& outputImage) {
    auto windowB = std::array<uint8_t, RADIUS_X * RADIUS_Y>();
    auto windowR = std::array<uint8_t, RADIUS_X * RADIUS_Y>();
    auto windowG = std::array<uint8_t, RADIUS_X * RADIUS_Y>();
         
    int edgeX = floor(RADIUS_X / 2);
    int edgeY = floor(RADIUS_Y / 2);

    for (int x = edgeX + 1; x < image.size().width - edgeX - 1; x++) {
        for (int y = edgeY + 1; y < image.size().height - edgeY - 1; y++) {
            int i = 0;
            for (int wx = 0; wx < RADIUS_X; wx++) {
                for (int wy = 0; wy < RADIUS_Y; wy++) {
                    auto pixel = image.at<cv::Vec3i>(x + wx - edgeX, y + wy - edgeY);
                    windowB[i] = pixel[0];
                    windowG[i] = pixel[1];
                    windowR[i] = pixel[2];
                    i++;
                }
            }
            std::sort(windowB.begin(), windowB.end());
            std::sort(windowG.begin(), windowG.end());
            std::sort(windowR.begin(), windowR.end());
            auto& pixel = outputImage.at<cv::Vec3i>(x, y);
            pixel[0] = windowB[RADIUS_X * RADIUS_Y / 2];
            pixel[1] = windowG[RADIUS_X * RADIUS_Y / 2];
            pixel[2] = windowR[RADIUS_X * RADIUS_Y / 2];
        }
    }
}

// Entry point for the program
int main(int argc, char** argv) {
    cv::Mat_<cv::Vec3i> image = cv::imread(argv[1], cv::IMREAD_ANYCOLOR);

    if(image.empty()) {
        std::cerr << "Error: Could not open or find the image" << std::endl;
        return -1;
    }

    srand(time(NULL));

    char* inputPath = argv[1];
    char* outputPath = argv[2];
    
    bool corrupt = false;
    int corruptionPercentage = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--corrupt") == 0 || strcmp(argv[i], "-c") == 0) {
            corrupt = true;
            corruptionPercentage = atoi(argv[i + 1]);
        }
    }

    if (corrupt) {
        auto corruptedImage = image.clone();
        corruptImage(image, corruptedImage, corruptionPercentage);
        cv::imwrite(outputPath, corruptedImage);
        return 0;
    }
    
    auto outputImage = image.clone();
    medianFilter(image, outputImage);

    print("Done");

    cv::imwrite(argv[2], outputImage);

    return 0;
}
