cmake_minimum_required(VERSION 3.4)
project(maki-engine)

if(WIN32)
	include("cmake/windows.cmake")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	include("cmake/mac.cmake")
else()
	include("cmake/linux.cmake")
endif()

set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/out")

add_subdirectory("deps/sdl2")
add_subdirectory("src")
