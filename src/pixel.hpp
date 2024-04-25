#ifndef PIXEL_H
#define PIXEL_H

#include <opencv2/core/hal/interface.h>
#include <opencv2/core/matx.hpp>
#include <iostream>

typedef cv::Vec3b Pixel;
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

/// calculate the luminosity of a BGR pixel
inline double luminosity(const BGRPixel &pixel) {
    return 0.299 * pixel.pixel.r + 0.587 * pixel.pixel.g + 0.114 * pixel.pixel.b;
}
inline double luminosity(const Pixel &pixel) {
    return 0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0];
}

#endif // PIXEL_H
