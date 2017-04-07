cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(NES_EMU)

add_executable(NES_EMU common_cpu.h common.h common_unit_test.c common_unit_test.h
               cpu_basic_operations.c cpu_basic_operations.h 
               cpu_basic_operations_unit_tests.c cpu_basic_operations_unit_tests.h
               cpu.c cpu_decode_logic.c cpu_decode_logic.h cpu_decode_logic_unit_tests.c
               cpu_decode_logic_unit_tests.h cpu.h cpu_instructions.c cpu_instructions.h
               cpu_unit_tests.c cpu_unit_tests.h main.c mappers.c mappers.h
               memory_operations.c memory_operations.h sdl_interface.c sdl_interface.h)

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

INCLUDE_DIRECTORIES(NES_EMU ${SDL2_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(NES_EMU ${SDL2_LIBRARIES})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")
