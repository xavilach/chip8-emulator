cmake_minimum_required(VERSION 3.0)

set(PROJECT_NAME chip8-emulator)

project(chip8-emulator)

set(SOURCES main.c cpu.c)
set(HEADERS cpu.h log.h)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(SDL2 REQUIRED)
find_package(Threads)

include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
target_link_libraries(${PROJECT_NAME} Threads::Threads)
