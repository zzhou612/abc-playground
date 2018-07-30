set(ectl_include ${PROJECT_SOURCE_DIR}/ectl)
set(ectl_include ${PROJECT_SOURCE_DIR}/ectl PARENT_SCOPE)

include_directories(${ectl_include})
include_directories(${ext_include})

add_definitions(-Wall -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-but-set-variable)
add_definitions(-DABC_NAMESPACE=abc)

file(GLOB ectl_source_files "*.h" "*.cpp")

add_library(ectl STATIC ${ectl_source_files})
