cmake_minimum_required(VERSION 3.4)
project(maki-engine-core)

include_directories("${sdl2_includes}")
link_directories("${sdl2_libs}")

# Copy the sdl dll into our binary output directory so that pbtool
# can be run directly without having to "install" first.
#configure_file("${sdl2_libs}/SDL2.dll" "." COPYONLY)
#configure_file("${sdl2_ttf_libs}/SDL2_ttf.dll" "." COPYONLY)
#configure_file("${sdl2_ttf_libs}/zlib1.dll" "." COPYONLY)
#configure_file("${sdl2_ttf_libs}/libfreetype-6.dll" "." COPYONLY)
#file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ".")

include_directories(".")
file(GLOB_RECURSE core_sources "core/*.cpp" "core/*.h")
add_library(maki_engine_core SHARED ${core_sources})
target_link_libraries(maki_engine_core SDL2)

install(TARGETS maki_engine_core DESTINATION ".")
#install(CODE "file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_INSTALL_PREFIX})")
