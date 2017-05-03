cmake_minimum_required(VERSION 3.4)
project(maki-engine-core)

include_directories(".")

include_directories("${sdl2_includes}")
link_directories("${sdl2_libs}")


file(GLOB_RECURSE core_sources "core/*.cpp" "core/*.h")
add_library(core SHARED ${core_sources})
target_link_libraries(core SDL2)

file(GLOB_RECURSE d3d_sources "d3d/*.cpp" "d3d/*.h")
add_library(d3d SHARED ${d3d_sources})
target_link_libraries(d3d core)



install(TARGETS core DESTINATION ".")
