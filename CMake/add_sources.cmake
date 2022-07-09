function(add_target_source target)
    add_source_files(${target} ${CMAKE_CURRENT_SOURCE_DIR}/Source ${CMAKE_CURRENT_SOURCE_DIR} ${ARGN})
endfunction()

function(add_source_files target source_dir base_dir)
    get_files_list(${source_dir} file_list)

    foreach (ignore_dir ${ARGN})
        list(FILTER file_list EXCLUDE REGEX ^${ignore_dir}*)
    endforeach()

    target_sources(${target} PRIVATE
        ${file_list}
    )
    add_source_files_list(${base_dir} ${file_list})

endfunction()

function(add_source_files_list base_dir)
    if(${ARGC} LESS 2)
        return()
    endif()

    list(APPEND files ${ARGV})
    list(REMOVE_ITEM files ${base_dir})
    # Group source files in the same structure as the file system
    foreach (FILE_LIST ${files})
        foreach(FILE_PATH ${FILE_LIST})
            # Always start with absolute path
            get_filename_component(ABSOLUTE_FILE_PATH ${FILE_PATH} ABSOLUTE)
            # Get relative path (to base dir)
            file(RELATIVE_PATH FILE_PATH ${base_dir} ${ABSOLUTE_FILE_PATH})
            # Extract file name/directory
            get_filename_component(FILE_NAME ${FILE_PATH} NAME)
            get_filename_component(FILE_DIRECTORY ${FILE_PATH} DIRECTORY)

            if("${FILE_NAME}" STREQUAL "")
                message(FATAL_ERROR "Bad file format")
            endif()

            # Get hierarchical structure to build source_groups
            set(FULL_GROUP_NAME "${SOURCE_GROUP_BASENAME}")
            if(NOT "${FILE_DIRECTORY}" STREQUAL "")
                string(REPLACE "/" "\\" GROUP_NAME ${FILE_DIRECTORY})
                set(FULL_GROUP_NAME "${SOURCE_GROUP_BASENAME}\\\\${GROUP_NAME}")
            endif()
            source_group("${FULL_GROUP_NAME}" FILES ${ABSOLUTE_FILE_PATH})
        endforeach()
    endforeach()
endfunction()

function(get_files_list source_dir output_variable)
    file(GLOB_RECURSE SRC_H     ${source_dir}/*.h)
    file(GLOB_RECURSE SRC_HPP   ${source_dir}/*.hpp)
    file(GLOB_RECURSE SRC_HH    ${source_dir}/*.hh)

    list(APPEND HEADERS_LIST ${SRC_H} ${SRC_HPP}  ${SRC_HH})

    list(LENGTH ARGN num_extra_args)

    if (num_extra_args EQUAL 0)
        file(GLOB_RECURSE SRC_C     ${source_dir}/*.c)
        file(GLOB_RECURSE SRC_CPP   ${source_dir}/*.cpp)
        file(GLOB_RECURSE SRC_CC    ${source_dir}/*.cc)
        
        if(APPLE)
            file(GLOB_RECURSE SRC_MM    ${source_dir}/*.mm)
            list(APPEND SOURCES_LIST ${SRC_MM})
        endif()

        list(APPEND SOURCES_LIST ${SRC_C} ${SRC_CPP} ${SRC_CC})
    endif()

    list(APPEND FILES_LIST ${HEADERS_LIST})

    if (SOURCES_LIST)
        # Remove cpp files for other OSs
        if (APPLE)
            set(remove_string "/windows/|/linux/")
        elseif(WIN32)
            set(remove_string "/macos/|/linux/")
        else()
            set(remove_string "/macos/|/windows/")
        endif()

        list(FILTER SOURCES_LIST EXCLUDE REGEX ".*${remove_string}.*")

        list(APPEND FILES_LIST ${SOURCES_LIST})
    endif()

    set(${output_variable} ${FILES_LIST} PARENT_SCOPE)

endfunction()