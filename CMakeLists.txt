cmake_minimum_required(VERSION 3.9)
project(abc-playground)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

add_subdirectory(ext)
add_subdirectory(ectl)
add_subdirectory(playground)
