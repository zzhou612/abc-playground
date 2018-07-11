include(ExternalProject)

set(ext_include ${PROJECT_SOURCE_DIR}/ext-lib/include)
set(ext_include ${PROJECT_SOURCE_DIR}/ext-lib/include PARENT_SCOPE)

set(ext_lib ${PROJECT_SOURCE_DIR}/ext-lib/lib)
set(ext_lib ${PROJECT_SOURCE_DIR}/ext-lib/lib PARENT_SCOPE)

set(abc_src ${CMAKE_BINARY_DIR}/ext-lib/install_abc-prefix/src)

file(MAKE_DIRECTORY ${ext_include})
file(MAKE_DIRECTORY ${ext_lib})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(compiler g++-6)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(compiler g++)
endif ()

message("Compiling abc with ${compiler}")

externalproject_add(install_abc
        DOWNLOAD_DIR .
        GIT_REPOSITORY https://github.com/berkeley-abc/abc.git
        CONFIGURE_COMMAND ""
        BUILD_COMMAND make CC=${compiler} CXX=${compiler} ABC_USE_NAMESPACE=abc libabc.a
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND rsync --include "*.h" --filter "hide,! */" -avm ${abc_src}/install_abc/src/ ${ext_include}/
        COMMAND cp ${abc_src}/install_abc/libabc.a ${ext_lib}/libabc.a
        LOG_DOWNLOAD 1
        LOG_BUILD 1
        LOG_INSTALL 1)

set(abc_libraries ${ext_lib}/libabc.a dl pthread readline PARENT_SCOPE)
