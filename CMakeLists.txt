cmake_minimum_required(VERSION 3.17)

project(wayland_learn)

option(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")
set(target_libs "-lwayland-client -lwayland-egl -lwayland-cursor -lxkbcommon -lEGL -lGL -lrt" CACHE STRING "Compile args")
set(target_name main)

message(STATUS "In root dir")

include(common.cmake)

# aux_source_directory(src/ main_srcs)
file(GLOB_RECURSE main_srcs src/*.c src/extra/*.c)

add_subdirectory(examples/)
add_subdirectory(tests/)

add_executable(${target_name} main.c ${main_srcs})

target_compile_definitions(${target_name} PUBLIC HAS_XDG_)

target_include_sub_directories_recursively(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(${target_name} PRIVATE ${target_libs})