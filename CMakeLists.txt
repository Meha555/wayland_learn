cmake_minimum_required(VERSION 3.17)

project(wayland_learn)

option(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")
set(target_libs "-lwayland-client -lwayland-egl -lwayland-cursor -lxkbcommon -lEGL -lGL -lrt" CACHE STRING "Compile args")
set(target_name main)

message(STATUS "In root dir")

function(target_include_sub_directories_recursively target prot root_dir)
	message("curdir: ${root_dir}")
    if (IS_DIRECTORY ${root_dir})               # 当前路径是一个目录吗，是的话就加入到包含目录
        #        if (${root_dir} MATCHES "include")
        message("include dir: " ${root_dir})
        target_include_directories(${target} ${prot} ${root_dir})
        #        endif()
    endif()

    file(GLOB subdir RELATIVE ${root_dir} ${root_dir}/*) # 获得当前目录下的所有文件
    message("subdir: ${subdir}")
	foreach(sub ${subdir})
        if (IS_DIRECTORY ${root_dir}/${sub}) # 如果是子目录，则递归地执行一遍
            target_include_sub_directories_recursively(${target} ${prot} ${root_dir}/${sub}) # 对子目录递归调用，包含
        endif()
    endforeach()
endfunction()

# aux_source_directory(src/ main_srcs)
file(GLOB_RECURSE main_srcs src/*.c src/extra/*.c)

add_subdirectory(examples/)

add_executable(${target_name} main.c ${main_srcs})

target_compile_definitions(${target_name} PUBLIC HAS_XDG_)

target_include_sub_directories_recursively(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(${target_name} PRIVATE ${target_libs})