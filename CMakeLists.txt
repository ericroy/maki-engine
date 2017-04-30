cmake_minimum_required(VERSION 3.4)
project(maki-engine)
include("cmake/build_settings.cmake")

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/out")


add_subdirectory("deps/sdl2")
add_subdirectory("src")
