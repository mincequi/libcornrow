cmake_minimum_required(VERSION 3.7)

project(cornrow)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -DG_DISABLE_ASSERT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Werror -Wno-old-style-cast")
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
find_package(ALSA REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)

add_library(${PROJECT_NAME} STATIC
    src/AlsaUtil.cpp
    src/Types.cpp

    src/audio/AlsaSink.cpp
    src/audio/AppSource.cpp
    src/audio/AudioBuffer.cpp
    src/audio/AudioConf.cpp
    src/audio/AudioConverter.cpp
    src/audio/AudioEncoderFfmpeg.cpp
    src/audio/AudioTypes.cpp
    src/audio/Crossover.cpp
    src/audio/FileSink.cpp
    src/audio/Loudness.cpp
    src/audio/Node.cpp
    src/audio/Peq.cpp
    src/audio/ScreamSource.cpp
    src/audio/SbcDecoder.cpp
    src/audio/Source.cpp
    src/audio/TBiquad.cpp
    src/core/AppSink.cpp
    src/core/Buffer.cpp
    src/core/Node.cpp
    src/core/Sink.cpp
    src/loguru/loguru.cpp
    src/rtp/RtpTypes.cpp
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
PRIVATE
    src
    include/coro
)

# Raspbery Pi specific libs
set(ENV{PKG_CONFIG_PATH} "/opt/vc/lib/pkgconfig")
pkg_check_modules(BCM_HOST bcm_host)
if(BCM_HOST_FOUND AND ENABLE_RPI)
    #add_definitions(-DTARGET_PI) # this is very old cmake
    target_compile_definitions(cornrow PUBLIC TARGET_PI)
    add_subdirectory(src/pi)
    set(CORO_LIBRARIES "cornrow-pi")
endif()

target_link_libraries(${PROJECT_NAME}
PUBLIC
    ${ALSA_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CORO_LIBRARIES}
    pthread
    dl
    sbc
    avutil
    avcodec
)

add_subdirectory(tests)
