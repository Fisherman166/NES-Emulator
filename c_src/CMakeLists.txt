cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(NES_EMU)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")

include_directories("./" "cpu/" "memory/" "unit_tests/" "ppu/")
add_subdirectory(memory)
add_subdirectory(cpu)
#add_subdirectory(ppu)
add_subdirectory(unit_tests)

set(src_files
    common.h
    main.c
    sdl_interface.h
    sdl_interface.c
)
add_executable(NES_EMU ${src_files})

# Find SDL
INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

INCLUDE_DIRECTORIES(NES_EMU ${SDL2_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(NES_EMU memory cpu unit_tests ${SDL2_LIBRARIES})
