cmake_minimum_required(VERSION 3.10)
project(CircleGreenLightDetect)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找OpenCV
find_package(OpenCV 4 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(detect_green_light detect_green_light.cpp)
target_link_libraries(detect_green_light ${OpenCV_LIBS})

add_executable(draw_green_circles draw_green_circles.cpp)
target_link_libraries(draw_green_circles ${OpenCV_LIBS})
