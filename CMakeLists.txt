cmake_minimum_required(VERSION 3.17)

project(wayland_demo)

option(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")
set(target_libs "-lwayland-client -lwayland-egl -lwayland-cursor -lxkbcommon -lEGL -lGL -lrt" CACHE STRING "Compile args")

function(add_test_file TAR_NAME TAR_FILE TAR_LIB)
message(NOTICE "${TAR_NAME} ${TAR_FILE}")
	get_filename_component(file_basename "${TAR_NAME}" NAME_WLE)
    add_executable(${file_basename} ${TAR_NAME} ${TAR_FILE})
	target_include_directories(${file_basename} PRIVATE inc)
	target_link_libraries(${file_basename} ${target_libs}) #wayland-client wayland-egl EGL rt
endfunction()

aux_source_directory(. sources)
aux_source_directory(src xdg_src)
file(GLOB_RECURSE headers inc/*.h)

foreach(file ${sources})
	add_test_file(${file} ${xdg_src} ${target_libs})
endforeach()