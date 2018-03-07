cmake_minimum_required(VERSION 3.9)

add_definitions(-Wall -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-but-set-variable)
add_definitions(-DABC_NAMESPACE=abc)

include_directories(${ext_include})
link_directories(${ext_lib})

add_executable(abc_playground main.cpp abc_api.h)
target_link_libraries(abc_playground ${abc_libraries})