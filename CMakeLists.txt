cmake_minimum_required(VERSION 3.20)
project(kuwahara LANGUAGES CXX)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the -O3 optimization flag and other flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Wpedantic -flto -ffast-math")

# Find OpenMP and enable it
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    set(OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS}") # Add additional flags if needed
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_executable(kuwahara src/main.cpp)
target_link_libraries(kuwahara PUBLIC ${OpenCV_LIBS} OpenMP::OpenMP_CXX)
