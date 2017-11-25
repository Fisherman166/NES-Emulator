cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(NES_EMU)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c99 -Wall")
#add_definitions(-DDEBUG)

include_directories("./include/")
add_subdirectory(src)

set(src_files
    src/main.c
)

add_executable(NES_EMU ${src_files})

# Find SDL
INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(NES_EMU src ${SDL2_LIBRARIES})
