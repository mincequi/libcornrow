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
    src/Types.cpp
    src/Util.cpp

    src/gstalsapassthroughsink.c
    src/gstavdtpsrc.cpp
    src/gstfdsrc.cpp

    src/audio/AlsaSink.cpp
    src/audio/AppSource.cpp
    src/audio/AudioBuffer.cpp
    src/audio/AudioConf.cpp
    src/audio/AudioConverter.cpp
    src/audio/AudioTypes.cpp
    src/audio/Loudness.cpp
    src/audio/Node.cpp
    src/audio/Peq.cpp
    src/audio/SbcDecoder.cpp
    src/audio/Source.cpp
    src/audio/TBiquad.cpp
    src/core/AppSink.cpp
    src/core/AppSource.cpp
    src/core/Buffer.cpp
    src/core/FdSource.cpp
    src/core/Node.cpp
    src/core/Sink.cpp
    src/loguru/loguru.cpp
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
    #xxhash
    dl
    sbc
    ao
)

target_compile_options(${PROJECT_NAME} PUBLIC ${GSTMM_CFLAGS_OTHER})

add_subdirectory(tests)
