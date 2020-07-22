cmake_minimum_required (VERSION 2.8)

project (video-sink)

SET(CMAKE_CXX_STANDARD 11)

find_package (Threads)
find_package (PkgConfig REQUIRED)
pkg_check_modules (GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0)

include_directories (${GSTREAMER_INCLUDE_DIRS})
link_directories (${GSTREAMER_LIBRARY_DIRS})

add_executable (record-test main.cpp videosink.cpp)
add_definitions (${GSTREAMER_CFLAGS_OTHER})
target_link_libraries (record-test ${GSTREAMER_LIBRARIES} Threads::Threads)
