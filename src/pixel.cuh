#ifndef PIXEL_H
#define PIXEL_H

typedef unsigned char uchar;
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

#endif // PIXEL_H
