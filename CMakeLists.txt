cmake_minimum_required(VERSION 3.7)

project(cornrow)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -DG_DISABLE_ASSERT -DUSE_KISS_FFT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -Werror -Wno-old-style-cast -DUSE_KISS_FFT -DLOGURU_WITH_STREAMS")
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
find_package(ALSA)
#find_package(asio REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(OpenSSL)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(PORTAUDIO REQUIRED portaudiocpp)

list(APPEND SOURCES
    src/Types.cpp

    src/audio/AudioAppSink.cpp
    src/audio/AudioCaps.cpp
    src/audio/AudioConf.cpp
    src/audio/AudioConverter.cpp
    src/audio/AudioDecoderFfmpeg.cpp
    src/audio/AudioDeviceInfo.cpp
    src/audio/AudioEncoderFfmpeg.cpp
    src/audio/AudioNode.cpp
    #src/audio/AudioTestSource.cpp
    src/audio/AudioTypes.cpp
    src/audio/Crossover.cpp
    src/audio/FileSink.cpp
    src/audio/Loudness.cpp
    src/audio/Peq.cpp
    src/audio/PortAudioSink.cpp
    src/audio/ScreamSource.cpp
    src/audio/TBiquad.cpp
    src/core/AppSink.cpp
    src/core/AppSource.cpp
    src/core/Buffer.cpp
    src/core/BufferPool.cpp
    src/core/FdSource.cpp
    src/core/Mainloop.cpp
    src/core/MainloopPrivate.cpp
    src/core/Node.cpp
    src/core/Sink.cpp
    src/core/Source.cpp
    src/core/SourceSelector.cpp
    src/core/TcpClientSink.cpp
    src/core/UdpSource.cpp
    src/core/Util.cpp
    src/loguru/loguru.cpp
    src/rtp/RtpDecoder.cpp
    src/rtp/RtpTypes.cpp
    src/rtsp/RtspServer.cpp
    src/rtsp/RtspServerPrivate.cpp
    src/rtsp/RtspMessage.cpp
    src/rtsp/RtspMessageHandler.cpp
    src/sdp/Sdp.cpp
    src/zeroconf/ZeroConfServer.cpp
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    list(APPEND SOURCES
        src/audio/SbcDecoder.cpp
    )
    list(APPEND LIBRARIES
        sbc
    )
endif()

if(ALSA_FOUND)
    list(APPEND SOURCES
        src/audio/AlsaSink.cpp
    )
endif()

if(OpenSSL_FOUND)
    list(APPEND SOURCES
        src/airplay/Airplay2Source.cpp
        src/airplay/Airplay2SourcePrivate.cpp
        src/airplay/AirplayDecrypter.cpp
        src/airplay/AirplaySource.cpp
        src/airplay/AirplaySourcePrivate.cpp
        src/airplay/AirplayRtpSession.cpp
        src/airplay/AirplayRtspMessageHandler.cpp
        src/airplay/AlacDecoder.cpp
        src/airplay/alac.c
    )
endif()

add_library(${PROJECT_NAME} STATIC
    ${SOURCES}
    #thirdparty/Gist/src/Gist.cpp
    #thirdparty/Gist/src/core/CoreFrequencyDomainFeatures.cpp
    #thirdparty/Gist/src/core/CoreTimeDomainFeatures.cpp
    #thirdparty/Gist/src/fft/WindowFunctions.cpp
    #thirdparty/Gist/src/onset-detection-functions/OnsetDetectionFunction.cpp
    #thirdparty/Gist/src/mfcc/MFCC.cpp
    #thirdparty/Gist/src/pitch/Yin.cpp
    #thirdparty/Gist/libs/kiss_fft130/kiss_fft.c
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    thirdparty/Gist/src
    thirdparty/Gist/libs/kiss_fft130

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

list(APPEND LIBRARIES
    ${ALSA_LIBRARIES}
    ${AVCODEC_LINK_LIBRARIES}
    ${AVUTIL_LINK_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CORO_LIBRARIES}
    ${PORTAUDIO_LINK_LIBRARIES}
    pthread
    dl
    dns_sd
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    ${LIBRARIES}
)

add_subdirectory(tests)
