project(cornrow-pi)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOMX_SKIP64BIT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOMX_SKIP64BIT")

add_library(${PROJECT_NAME} STATIC
    PiHdmiAudioSink.cpp
    audio.c
    ilclient.c
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    coro
    ${BCM_HOST_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    cornrow
    ${BCM_HOST_LINK_LIBRARIES}
    /opt/vc/lib/libopenmaxil.so
)
