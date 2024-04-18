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

function(add_test_file TAR_NAME TAR_FILE TAR_LIB TAR_INC)
message(NOTICE "${TAR_NAME} ${TAR_FILE}")
	get_filename_component(file_basename "${TAR_NAME}" NAME_WLE)
    add_executable(${file_basename} ${TAR_NAME} ${TAR_FILE})
	target_include_directories(${file_basename} PRIVATE ${TAR_INC})
	target_link_libraries(${file_basename} ${TAR_LIB})
endfunction()