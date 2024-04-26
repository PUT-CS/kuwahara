cmake_minimum_required(VERSION 2.8)
project(kuwahara)
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Add the -O3 optimization flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Wpedantic")

add_executable(kuwahara src/main.cpp)
target_link_libraries(kuwahara ${OpenCV_LIBS})
