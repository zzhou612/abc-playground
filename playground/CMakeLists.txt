cmake_minimum_required(VERSION 3.9)

add_definitions(-Wall -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-but-set-variable)
add_definitions(-DABC_NAMESPACE=abc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(abc_libraries libabc dl pthread readline)

find_package(Boost REQUIRED COMPONENTS regex system filesystem timer)

include_directories(${ectl_include})
include_directories(${abc_include})

add_executable(abc_playground main.cpp)
target_link_libraries(abc_playground
        ectl
        ${abc_libraries}
        ${Boost_LIBRARIES})
