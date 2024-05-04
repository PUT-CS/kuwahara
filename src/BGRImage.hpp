#ifndef BGR_IMAGE_H
#define BGR_IMAGE_H

#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include "pixel.cuh"

typedef cv::Vec3b Pixel;

/// calculate the luminosity of a BGR pixel
inline double luminosity(const Pixel& pixel) {
    return 0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0];
}

inline BGRPixel BGRfromPixel(const Pixel& pixel) {
    return {static_cast<uchar>(pixel[0]),
            static_cast<uchar>(pixel[1]),
            static_cast<uchar>(pixel[2]),
            static_cast<uchar>(luminosity(pixel))};
}

inline BGRPixel* intoBGRPixelArray1D(cv::Mat &image) {
    auto pixelArray = new BGRPixel[image.size().height * image.size().width];

    for (int i = 0; i < image.size().height; i++) {
        for (int j = 0; j < image.size().width; j++) {
            auto &pixel = image.at<Pixel>(i, j);

            pixelArray[i * image.size().width + j] = {static_cast<uchar>(pixel[0]),
                                                      static_cast<uchar>(pixel[1]),
                                                      static_cast<uchar>(pixel[2]),
                                                      static_cast<uchar>(luminosity(pixel))};
        }
    }
    return pixelArray;
}

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

inline cv::Mat fromBGRPixelArray1D(BGRPixel *pixelArray, cv::Size size) {
    auto image = cv::Mat(size, CV_8UC3);
    for (int i = 0; i < size.height; i++) {
        for (int j = 0; j < size.width; j++) {
            auto &pixel = image.ptr<Pixel>(i)[j];
            pixel[0] = pixelArray[i * size.width + j].data[0];
            pixel[1] = pixelArray[i * size.width + j].data[1];
            pixel[2] = pixelArray[i * size.width + j].data[2];
        }
    }
    return image;
}

inline cv::Mat fromBGRPixelArray(BGRPixel **pixelArray, cv::Size size) {
    auto image = cv::Mat(size, CV_8UC3);
    for (int i = 0; i < size.height; i++) {
        for (int j = 0; j < size.width; j++) {
            auto &pixel = image.ptr<Pixel>(i)[j];
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

inline BGRPixel* allocateBGRPixelArray1D(cv::Size size) {
    return new BGRPixel[size.height * size.width];
}

inline void freeBGRPixelArray1D(BGRPixel *pixelArray) { delete[] pixelArray; }

inline void freeBGRPixelArray(BGRPixel **pixelArray, cv::Size size) {
    for (int i = 0; i < size.height; i++) {
        delete[] pixelArray[i];
    }
    delete[] pixelArray;
}

#endif //BGR_IMAGE_H
