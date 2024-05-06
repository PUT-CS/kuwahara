#include "BGRImage.hpp"
#include "pixel.cuh"
#include "print.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <string>
#include <vector>
#include "kernel.cuh"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

// Entry point for the program
int main(int argc, char **argv) {
    auto inputPath = argv[1];
    auto outputPath = argv[2];
    if (argc < 3) {
        std::cerr << "Usage: kuwahara <input_image> <output_image> [--window window_size]"
                  << std::endl;
        return -1;
    }

    int windowSize = 9;
    if (argv[3] != nullptr && argv[4] != nullptr && std::string(argv[3]) == "--window") {
        windowSize = std::stoi(argv[4]);
        if (windowSize % 2 == 0) {
            std::cerr << "Error: Window size must be an odd number" << std::endl;
            return -1;
        }
    }

    cv::Mat bgrImage = cv::imread(inputPath, cv::IMREAD_COLOR);
    auto size = bgrImage.size();

    if (bgrImage.empty()) {
        std::cerr << "Error: Could not open or find the image" << std::endl;
        return -1;
    }

    // BGRPixel **pixels = intoBGRPixelArray(bgrImage);
    // BGRPixel **outputPixels = allocateBGRPixelArray(size);
    BGRPixel *pixels = intoBGRPixelArray1D(bgrImage);
    BGRPixel *outputPixels = allocateBGRPixelArray1D(size);

    int quadrantSize = std::ceil(windowSize / 2.0);

    BGRPixel* devicePixels;
    BGRPixel* deviceOutputPixels; 

    int imageSize = size.width * size.height * sizeof(BGRPixel);
    
    // allocate to the pointers
    cudaMalloc((void**) &devicePixels, imageSize);
    cudaMalloc((void**) &deviceOutputPixels, imageSize);

    // copy image data to the device
    cudaMemcpy(devicePixels, pixels, imageSize, cudaMemcpyHostToDevice);
    cudaMemcpy(deviceOutputPixels, outputPixels, imageSize, cudaMemcpyHostToDevice);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    KernelWrapper::launchKuwaharaKernel(devicePixels, deviceOutputPixels, size.width, size.height, quadrantSize);
    cudaDeviceSynchronize();
    
    auto end = std::chrono::high_resolution_clock::now();
    
    //cudaError_t errorCode = cudaGetLastError();
    //printf("CUDA error: %s\n", cudaGetErrorString(errorCode));
    
    cudaMemcpy(outputPixels, deviceOutputPixels, imageSize, cudaMemcpyDeviceToHost);

    std::chrono::duration<double> elapsed = end - start;
    print(elapsed.count());

    auto outputMat = fromBGRPixelArray1D(outputPixels, size);
    cv::imwrite(outputPath, outputMat);

    delete pixels;
    delete outputPixels;
    cudaFree(devicePixels);
    cudaFree(deviceOutputPixels);

    return 0;
}
