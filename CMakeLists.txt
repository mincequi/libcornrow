cmake_minimum_required(VERSION 3.10)

# Required cmake modules
include(ExternalProject)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GST REQUIRED gstreamer-1.0)

# Add gstreamermm
set(GSTMM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/gstreamermm)
ExternalProject_Add(project_gstreamermm
    SOURCE_DIR          ${GSTMM_SOURCE_DIR}
    CONFIGURE_COMMAND   ./autogen.sh --enable-static --disable-shared --disable-documentation --disable-unittests --disable-deprecated-api --prefix=${CMAKE_CURRENT_BINARY_DIR}
    BUILD_COMMAND       make
    BUILD_IN_SOURCE     1
    INSTALL_COMMAND     make install
)

# gstreamermm-dsp
add_library(gstreamermm-dsp STATIC
    src/gstreamermm-dsp.cpp
)

target_include_directories(gstreamermm-dsp
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
PRIVATE
    src
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/gstreamermm/gstreamer
    ${GLIBMM_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
)

target_link_libraries(gstreamermm-dsp
    gstreamermm
    ${GLIBMM_LIBRARIES}
)

add_dependencies(gstreamermm-dsp project_gstreamermm)
