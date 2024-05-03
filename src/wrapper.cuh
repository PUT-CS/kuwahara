#pragma once

namespace KernelWrapper {
  void launchKuwaharaKernel(BGRPixel **image, BGRPixel **outputImage, int sizeX, int sizeY, int quadrantSize);
}