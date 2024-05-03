cmake_minimum_required(VERSION 3.20)
project(kuwahara LANGUAGES CXX CUDA)  # Add CUDA lang
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the -O3 optimization flag and other flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Wpedantic -flto -ffast-math")

# Add CUDA flags and set architecture
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -O3 -arch=sm_70")

# Add CUDA files to the project
file(GLOB CUDA_SOURCES src/*.cu)
find_package(CUDA  REQUIRED)
include_directories("${CUDA_INCLUDE_DIRS}")

# Add executable with CUDA sources
add_executable(kuwahara src/main.cpp ${CUDA_SOURCES})

# Link CUDA libraries
target_link_libraries(kuwahara PUBLIC ${OpenCV_LIBS})
