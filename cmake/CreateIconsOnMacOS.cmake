function (make_icns_from_png)
        cmake_parse_arguments (
                ARG
                ""             # Boolean args
                "INPUT;OUTPUT" # List of single-value args
                ""             # Multi-valued args
                ${ARGN})

        find_program (
                sips
                NAMES "sips"
                DOC "Path to sips")
        if (NOT sips)
                message (FATAL_ERROR "Could not find 'sips' - are you on macOS?")
        endif ()

        get_filename_component (ARG_INPUT_ABS "${ARG_INPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        get_filename_component (ARG_INPUT_ABS_BIN "${ARG_INPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        get_filename_component (ARG_INPUT_FN "${ARG_INPUT_ABS_BIN}" NAME_WE)
        get_filename_component (ARG_INPUT_DIR "${ARG_INPUT_ABS_BIN}" DIRECTORY)

        set (sourceimg "${ARG_INPUT_ABS}")

        set (basepath "${ARG_INPUT_DIR}/${ARG_INPUT_FN}")
        set (output_icns "${basepath}.icns")
        set (iconset "${basepath}.iconset")

        set (deplist "")

        foreach (size IN ITEMS 16 32 128 256 512)
                math (EXPR size2x "2 * ${size}")

                set (ipath "${iconset}/icon_${size}x${size}.png")
                set (ipath2x "${iconset}/icon_${size}x${size}@2x.png")

                list (APPEND deplist "${ipath}" "${ipath2x}")

                add_custom_command (
                        OUTPUT "${ipath}"
                        COMMAND "${convert_exe}" ARGS "${sourceimg}" --resampleHeightWidth ${size} ${size} --out "${ipath}"
                        MAIN_DEPENDENCY "${sourceimg}"
                        COMMENT "ICNS resize: ${ipath}"
                        VERBATIM)

                add_custom_command (
                        OUTPUT "${ipath2x}"
                        COMMAND "${convert_exe}" ARGS "${sourceimg}" --resampleHeightWidth ${size2x} ${size2x} --out "${ipath2x}"
                        MAIN_DEPENDENCY "${sourceimg}"
                        COMMENT "ICNS resize: ${ipath2x}"
                        VERBATIM)
        endforeach ()

        add_custom_command (
                OUTPUT "${output_icns}"
                COMMAND iconutil ARGS --convert icns --output "${output_icns}" "${iconset}"
                MAIN_DEPENDENCY "${sourceimg}"
                DEPENDS ${deplist}
                COMMENT "ICNS: ${output_icns}"
                VERBATIM)

        if (ARG_OUTPUT)
                set ("${ARG_OUTPUT}" "${output_icns}" PARENT_SCOPE)
        endif ()
endfunction ()

function (make_icns)
        cmake_parse_arguments (
                ARG
                ""             # Boolean args
                "INPUT;OUTPUT" # List of single-value args
                ""             # Multi-valued args
                ${ARGN})

        if (NOT ARG_INPUT)
                message (FATAL_ERROR "INPUT is required")
        endif ()

        if (NOT IS_ABSOLUTE "${ARG_INPUT}")
                get_filename_component (ARG_INPUT "${ARG_INPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        endif ()

        if (NOT EXISTS "${ARG_INPUT}")
                message (FATAL_ERROR "INPUT does not exist: ${ARG_INPUT}")
        endif ()

        file (RELATIVE_PATH ARG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}" "${ARG_INPUT}")

        get_filename_component (ARG_INPUT_EXT "${ARG_INPUT}" EXT)
        if ("${ARG_INPUT_EXT}" STREQUAL ".png")
                make_icns_from_png (INPUT "${ARG_INPUT}" OUTPUT child_output)
        else ()
                message (FATAL_ERROR "INPUT must refer to a .png, but a ${ARG_INPUT_EXT} was provided")
        endif ()

        if (ARG_OUTPUT)
                set ("${ARG_OUTPUT}" "${child_output}" PARENT_SCOPE)
        endif ()
endfunction ()
