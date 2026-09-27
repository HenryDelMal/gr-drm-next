cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
project(gr-drm VERSION 0.1.0 LANGUAGES C CXX)
enable_testing()

list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/cmake/Modules)
find_package(Gnuradio "3.10" REQUIRED COMPONENTS runtime fft)
find_package(Faac REQUIRED)
find_package(Faad REQUIRED)
find_package(Opus REQUIRED)

include(GrMinReq)
include(GrCompilerSettings)
include(GrVersion)
include(GrPlatform)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GR_INCLUDE_DIR include/drm)
set(GR_CMAKE_DIR lib${LIB_SUFFIX}/cmake/drm)
set(GR_PKG_DATA_DIR ${GR_DATA_DIR}/${CMAKE_PROJECT_NAME})
set(GR_PKG_DOC_DIR ${GR_DOC_DIR}/${CMAKE_PROJECT_NAME})

if(APPLE)
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${GR_LIBRARY_DIR})
endif()

add_subdirectory(include/drm)
add_subdirectory(lib)
if(ENABLE_PYTHON)
    add_subdirectory(python)
    add_subdirectory(grc)
endif()
add_subdirectory(apps)

configure_file(${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake @ONLY)
add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
