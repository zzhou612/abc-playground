cmake_minimum_required(VERSION 3.9)

add_definitions(-Wall -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-but-set-variable)
add_definitions(-DABC_NAMESPACE=abc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

find_package(Boost REQUIRED COMPONENTS regex system filesystem timer)

include_directories(${ectl_include})
include_directories(${ext_include})
link_directories(${ext_lib})

file(GLOB dals_source_files "*.h" "*.cpp")

add_executable(dals ${dals_source_files})
target_link_libraries(dals
        ectl
        ${abc_libraries}
        ${Boost_LIBRARIES})
