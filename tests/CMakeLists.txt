message(STATUS "In tests dir")

include(../common.cmake)

option(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(target_libs "-lwayland-client -lwayland-egl -lEGL -lGL -lrt" CACHE STRING "Compile args")


aux_source_directory(./ sources)
aux_source_directory(../src/extra/ xdg_src)
# file(GLOB_RECURSE headers ../inc/*.h)

foreach(file ${sources})
	add_test_file(${file} ${xdg_src} ${target_libs} ../inc)
endforeach()