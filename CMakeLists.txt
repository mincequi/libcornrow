cmake_minimum_required(VERSION 3.10)

project(gstreamermm-dsp)

# Required cmake modules
include(ExternalProject)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-base-1.0
    gstreamer-audio-1.0
    gstreamer-video-1.0
    gstreamer-pbutils-1.0
    gstreamer-net-1.0
)

# Add gstreamermm
set(GSTMM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/gstreamermm)
ExternalProject_Add(project_gstreamermm
    SOURCE_DIR          ${GSTMM_SOURCE_DIR}
    CONFIGURE_COMMAND   ./autogen.sh --enable-static --disable-shared --disable-documentation --disable-unittests --prefix=${CMAKE_CURRENT_BINARY_DIR}
    BUILD_COMMAND       make
    BUILD_IN_SOURCE     1
    INSTALL_COMMAND     make install
)

# gstreamermm-dsp
add_library(${PROJECT_NAME} STATIC
    src/gstreamermm-dsp.cpp
    src/Biquad.cpp
    src/Crossover.cpp
    src/Loudness.cpp
    src/Peq.cpp
    src/Util.cpp
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${GLIBMM_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${GSTMM_SOURCE_DIR}/gstreamer
PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME}
    ${CMAKE_CURRENT_BINARY_DIR}/lib/libgstreamermm-1.0.a    # library order matters
    ${GST_LIBRARIES}
    ${GLIBMM_LIBRARIES}
)

add_dependencies(${PROJECT_NAME} project_gstreamermm)

add_subdirectory(tests)
