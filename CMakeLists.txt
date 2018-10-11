# Required cmake modules
include(ExternalProject)

file(DOWNLOAD
    #https://raw.githubusercontent.com/GNOME/gstreamermm/master/gstreamer/src/basetransform.ccg
)
#https://raw.githubusercontent.com/GNOME/gstreamermm/master/gstreamer/src/basetransform.hg

# Add gstreamermm as dependency
ExternalProject_Add(project_gstreamermm
    URL     https://ftp.gnome.org/pub/GNOME/sources/gstreamermm/1.10/gstreamermm-1.10.0.tar.xz
    CONFIGURE_COMMAND "./configure"
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
#add_library(luajit STATIC IMPORTED)
#set_property(TARGET luajit PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libluajit-5.1.a)
#include_directories(${install_dir}/include/luajit-2.0)

add_dependencies(gstreamermm project_gstreamermm)

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
)

target_link_libraries(gstreamermm-dsp
    gstreamermm
)

