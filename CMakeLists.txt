cmake_minimum_required(VERSION 3.7)

project(gstreamermm-dsp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTMM REQUIRED gstreamermm-1.0)

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
    ${GSTMM_INCLUDE_DIRS}
PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME} ${GSTMM_LIBRARIES})
target_compile_options(${PROJECT_NAME} PUBLIC ${GSTMM_CFLAGS_OTHER})

add_subdirectory(tests)
