function(create_icon_from_png)
    cmake_parse_arguments(
        ARG
        ""             # Boolean args
        "INPUT;OUTPUT" # List of single-value args
        ""             # Multi-valued args
        ${ARGN})

    find_program(
        SIPS
        NAMES "sips"
        DOC "Path to sips")
    if(NOT SIPS)
        message(FATAL_ERROR "Could not find 'sips' - only call this function on macOS.")
    endif()

    get_filename_component(ARG_INPUT_ABS "${ARG_INPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    get_filename_component(ARG_INPUT_ABS_BIN "${ARG_INPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    get_filename_component(ARG_INPUT_FN "${ARG_INPUT_ABS_BIN}" NAME_WE)
    get_filename_component(ARG_INPUT_DIR "${ARG_INPUT_ABS_BIN}" DIRECTORY)

    set(SOURCE_IMG "${ARG_INPUT_ABS}")
    set(BASE_PATH "${ARG_INPUT_DIR}/${ARG_INPUT_FN}")
    set(OUTPUT_ICNS "${BASE_PATH}.icns")
    set(ICON_SET_DIRECTORY "${BASE_PATH}.iconset")

    set(DEPENDENCIES "")

    foreach(size IN ITEMS 16 32 128 256 512)
        math(EXPR size2x "2 * ${size}")

        set(ICON_PATH "${ICON_SET_DIRECTORY}/icon_${size}x${size}.png")
        set(ICON_PATH2x "${ICON_SET_DIRECTORY}/icon_${size}x${size}@2x.png")

        list(APPEND DEPENDENCIES "${ICON_PATH}" "${ICON_PATH2x}")

        add_custom_command(
            OUTPUT "${ICON_PATH}"
            COMMAND ${CMAKE_COMMAND} -E make_directory ${ICON_SET_DIRECTORY}
            COMMAND "${SIPS}" ARGS "${SOURCE_IMG}" --resampleHeightWidth ${size} ${size} --out "${ICON_PATH}"
            MAIN_DEPENDENCY "${SOURCE_IMG}"
            COMMENT "ICNS resize: ${ICON_PATH}"
            VERBATIM)

        add_custom_command(
            OUTPUT "${ICON_PATH2x}"
            COMMAND ${CMAKE_COMMAND} -E make_directory ${ICON_SET_DIRECTORY}
            COMMAND "${SIPS}" ARGS "${SOURCE_IMG}" --resampleHeightWidth ${size2x} ${size2x} --out "${ICON_PATH2x}"
            MAIN_DEPENDENCY "${SOURCE_IMG}"
            COMMENT "ICNS resize: ${ICON_PATH2x}"
            VERBATIM)
    endforeach()

    add_custom_command(
        OUTPUT "${OUTPUT_ICNS}"
        COMMAND iconutil ARGS --convert icns --output "${OUTPUT_ICNS}" "${ICON_SET_DIRECTORY}"
        MAIN_DEPENDENCY "${sourceimg}"
        DEPENDS ${DEPENDENCIES}
        COMMENT "ICNS: ${OUTPUT_ICNS}"
        VERBATIM)

    if(ARG_OUTPUT)
        set("${ARG_OUTPUT}" "${OUTPUT_ICNS}" PARENT_SCOPE)
    endif()
endfunction()

function(create_app_icon_on_macos)
    cmake_parse_arguments(
        ARG
        ""             # Boolean args
        "INPUT;OUTPUT" # List of single-value args
        ""             # Multi-valued args
        ${ARGN})

    if(NOT ARG_INPUT)
        message(FATAL_ERROR "INPUT is required")
    endif()

    if(NOT IS_ABSOLUTE "${ARG_INPUT}")
        get_filename_component(ARG_INPUT "${ARG_INPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    if(NOT EXISTS "${ARG_INPUT}")
        message(FATAL_ERROR "INPUT does not exist: ${ARG_INPUT}")
    endif()

    file(RELATIVE_PATH ARG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}" "${ARG_INPUT}")

    get_filename_component(ARG_INPUT_EXT "${ARG_INPUT}" EXT)
    if("${ARG_INPUT_EXT}" STREQUAL ".png")
        create_icon_from_png(INPUT "${ARG_INPUT}" OUTPUT child_output)
    else()
        message(FATAL_ERROR "INPUT must refer to a .png, but a ${ARG_INPUT_EXT} was provided")
    endif()

    if(ARG_OUTPUT)
        set("${ARG_OUTPUT}" "${child_output}" PARENT_SCOPE)
    endif()
endfunction()
