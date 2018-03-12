include(ExternalProject)

set(ext_include ${PROJECT_SOURCE_DIR}/ext/include)
set(ext_include ${PROJECT_SOURCE_DIR}/ext/include PARENT_SCOPE)

set(ext_lib ${PROJECT_SOURCE_DIR}/ext/lib)
set(ext_lib ${PROJECT_SOURCE_DIR}/ext/lib PARENT_SCOPE)

set(abc_src ${CMAKE_BINARY_DIR}/ext/install_abc-prefix/src)

file(MAKE_DIRECTORY ${ext_include})
file(MAKE_DIRECTORY ${ext_lib})

externalproject_add(install_abc
        DOWNLOAD_DIR .
        HG_REPOSITORY https://bitbucket.org/alanmi/abc
        CONFIGURE_COMMAND ""
        BUILD_COMMAND make ABC_USE_NAMESPACE=abc libabc.a
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND rsync --include "*.h" --filter "hide,! */" -avm ${abc_src}/install_abc/src/ ${ext_include}/ COMMAND cp ${abc_src}/install_abc/libabc.a ${ext_lib}/libabc.a
        LOG_DOWNLOAD 1
        LOG_BUILD 1
        LOG_INSTALL 1)

set(abc_libraries ${ext_lib}/libabc.a dl pthread readline PARENT_SCOPE)
