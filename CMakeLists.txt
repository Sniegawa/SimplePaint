cmake_minimum_required(VERSION 3.16)

project(SimplePaintProject)


set(CMAKE_C_STANDARD 11)

add_subdirectory(libs/GLAD)
add_subdirectory(libs/glfw)

include_directories(libs/SimplePaint)
include_directories(libs/Nuklear)

add_subdirectory(src)

