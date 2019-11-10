cmake_minimum_required(VERSION 3.7)

project(gstreamermm-dsp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -DG_DISABLE_ASSERT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Werror")

find_package(PkgConfig REQUIRED)
find_package(ALSA REQUIRED)
pkg_check_modules(GSTMM REQUIRED gstreamermm-1.0)

# gstreamermm-dsp
add_library(${PROJECT_NAME} STATIC
    src/gstreamermm-dsp.cpp
    src/AlsaUtil.cpp
    src/Biquad.cpp
    src/Crossover.cpp
    src/Loudness.cpp
    src/Peq.cpp
    src/Types.cpp
    src/Util.cpp

    src/gstalsapassthroughsink.c
    src/gstavdtpsrc.cpp
    src/gstfdsrc.cpp

    src/core/AppSource.cpp
    src/core/Buffer.cpp
    src/core/FdSource.cpp
    src/rtp/RtpSbcDepay.cpp
    src/rtp/RtpTypes.cpp
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${GSTMM_INCLUDE_DIRS}
PRIVATE
    src
    include/coro
)

target_link_libraries(${PROJECT_NAME}
    ${GSTMM_LIBRARIES}
    ${ALSA_LIBRARIES}
    pthread
    xxhash
)

target_compile_options(${PROJECT_NAME} PUBLIC ${GSTMM_CFLAGS_OTHER})

add_subdirectory(tests)
