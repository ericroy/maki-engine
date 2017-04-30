cmake_minimum_required(VERSION 3.4)
project(sdl2)

# Export some paths that consumers can use
set(sdl2_includes "${CMAKE_CURRENT_SOURCE_DIR}/SDL2-2.0.5/include" PARENT_SCOPE)
set(sdl2_libs "${CMAKE_CURRENT_SOURCE_DIR}/SDL2-2.0.5/lib/x64" PARENT_SCOPE)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/SDL2-2.0.5/lib/x64/SDL2.dll" DESTINATION ".")
