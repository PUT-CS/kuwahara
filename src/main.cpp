#include "BGRImage.hpp"
#include "pixel.cuh"
#include "print.hpp"
#include "wrapper.cuh"
#include <array>
#include <cmath>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <string>
#include <vector>
#include "wrapper.cuh"
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

    BGRPixel **pixels = intoBGRPixelArray(bgrImage);
    BGRPixel **outputPixels = allocateBGRPixelArray(size);

    auto start = std::chrono::high_resolution_clock::now();
    int quadrantSize = std::ceil(windowSize / 2.0);

    BGRPixel** cudaPixels;
    BGRPixel** cudaOutputPixels; 

    int numberOfPixels = size.width * size.height;
    
    // allocate to the pointers
    cudaMalloc((BGRPixel**) &cudaPixels, numberOfPixels * sizeof(BGRPixel));
    cudaMalloc((BGRPixel**) &cudaOutputPixels, numberOfPixels * sizeof(BGRPixel));

    print(size.width, size.height);
    printf("Allocated memory for %d pixels\n", numberOfPixels);

    // copy image data to the device
    cudaMemcpy(cudaPixels, pixels, numberOfPixels * sizeof(BGRPixel), cudaMemcpyHostToDevice);
    cudaMemcpy(cudaOutputPixels, outputPixels, numberOfPixels * sizeof(BGRPixel), cudaMemcpyHostToDevice);
    
    KernelWrapper::launchKuwaharaKernel(pixels, outputPixels, size.width, size.height, quadrantSize);
    cudaError_t errorCode = cudaGetLastError();
    printf("CUDA error: %s\n", cudaGetErrorString(errorCode));
    cudaDeviceSynchronize();
    
    cudaMemcpy(outputPixels, cudaOutputPixels, numberOfPixels * sizeof(BGRPixel), cudaMemcpyDeviceToHost);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    print(elapsed.count());

    auto outputMat = fromBGRPixelArray(outputPixels, size);
    cv::imwrite(outputPath, outputMat);

    freeBGRPixelArray(pixels, size);
    freeBGRPixelArray(outputPixels, size);

    return 0;
}
