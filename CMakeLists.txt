project(GSVideoAudio)
cmake_minimum_required(VERSION 3.16)
set(CMAKE_BUILD_TYPE DEBUG)
#include(VideoAudioCapture.h)
message('q212222')
set(CMAKE_CXX_STANDARD 20)
find_package(spdlog REQUIRED)
find_package(OpenCV REQUIRED)
include_directories(/usr/include/spdlog)
include_directories(/usr/include/gstreamer-1.0)
include_directories(/usr/include/glib-2.0)
include_directories(/usr/include/c++/9)
include_directories(/usr/lib/aarch64-linux-gnu/glib-2.0/include)
include_directories(${OpenCV_INCLUDE_DIRS})
add_library(mygst-video-audio SHARED VideoAudioCapture.cpp URI.cpp VideoAudioCapture.h VideoAudioOption.h gstBufferManager.h gstBufferManager.cpp gstVideoAudioUtils.hpp)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} mygst-video-audio ${OpenCV_LIBS} gstreamer-1.0 gobject-2.0 glib-2.0 gstapp-1.0 spdlog::spdlog )
