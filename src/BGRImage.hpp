#ifndef BGR_IMAGE_H
#define BGR_IMAGE_H

#include "pixel.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>

inline BGRPixel **intoBGRPixelArray(cv::Mat &image) {
    auto pixelArray = new BGRPixel *[image.size().height];

    for (int i = 0; i < image.size().height; i++) {
        pixelArray[i] = new BGRPixel[image.size().width];
        for (int j = 0; j < image.size().width; j++) {
            auto &pixel = image.at<Pixel>(i, j);

            pixelArray[i][j] = {static_cast<uchar>(pixel[0]),
                                static_cast<uchar>(pixel[1]),
                                static_cast<uchar>(pixel[2]),
                                static_cast<uchar>(luminosity(pixel))};
        }
    }
    return pixelArray;
}

inline cv::Mat fromBGRPixelArray(BGRPixel **pixelArray, cv::Size size) {
    auto image = cv::Mat(size, CV_8UC3);
    for (int i = 0; i < size.height; i++) {
        for (int j = 0; j < size.width; j++) {
            auto &pixel = image.at<Pixel>(i, j);
            pixel[0] = pixelArray[i][j].data[0];
            pixel[1] = pixelArray[i][j].data[1];
            pixel[2] = pixelArray[i][j].data[2];
        }
    }
    return image;
}

inline BGRPixel** allocateBGRPixelArray(cv::Size size) {
    auto pixelArray = new BGRPixel *[size.height];
    for (int i = 0; i < size.height; i++) {
        pixelArray[i] = new BGRPixel[size.width];
    }
    return pixelArray;
}

inline void deallocateBGRPixelArray(BGRPixel **pixelArray, cv::Size size) {
    for (int i = 0; i < size.height; i++) {
        delete[] pixelArray[i];
    }
    delete[] pixelArray;
}

inline void printBGRPixelArray(BGRPixel **pixelArray, cv::Size size) {
    for (int i = 0; i < size.height; i++) {
        for (int j = 0; j < size.width; j++) {
            auto &pixel = pixelArray[i][j];
            std::cout << "(" << (int) pixel.data[0] << ", " << (int) pixel.data[1] << ", " << (int) pixel.data[2] << ") ";
        }
        std::cout << std::endl;
    }
}

#endif //BGR_IMAGE_H
