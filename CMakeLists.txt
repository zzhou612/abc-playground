cmake_minimum_required(VERSION 3.9)
project(abc_playground)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

set(ext_include ${PROJECT_SOURCE_DIR}/ext/include)
set(ext_lib ${PROJECT_SOURCE_DIR}/ext/lib)

file(MAKE_DIRECTORY ${ext_include})
file(MAKE_DIRECTORY ${ext_lib})

add_subdirectory(ext)
add_subdirectory(playground)