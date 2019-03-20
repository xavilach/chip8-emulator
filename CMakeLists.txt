cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME chip8-emulator)

project(chip8-emulator)

set(SOURCES main.c cpu.c)
set(HEADERS cpu.h)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
