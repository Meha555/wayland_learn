cmake_minimum_required(VERSION 3.17)

project(wayland_learn)

option(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")
set(target_libs "-lwayland-client -lwayland-egl -lwayland-cursor -lxkbcommon -lEGL -lGL -lrt" CACHE STRING "Compile args")
set(target_name main)

message(STATUS "In root dir")

aux_source_directory(src/ main_srcs)

add_subdirectory(examples/)

add_executable(${target_name} main.c ${main_srcs})
target_include_directories(${target_name} PRIVATE inc/)
target_link_libraries(${target_name} PRIVATE ${target_libs})