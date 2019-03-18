cmake_minimum_required(VERSION 3.0)

project(chip8-emulator)

set(SOURCES main.c cpu.c)
set(HEADERS cpu.h)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(chip8-emulator ${SOURCES} ${HEADERS})
target_link_libraries(chip8-emulator ${SDL2_LIBRARIES})
