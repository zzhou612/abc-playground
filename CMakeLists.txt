cmake_minimum_required(VERSION 3.9)
project(abc-playground)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    include_directories(/usr/local/include)
endif ()

add_subdirectory(ext-lib)
add_subdirectory(ectl)
add_subdirectory(playground)
add_subdirectory(dals)
