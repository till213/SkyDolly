set(TARGET_NAME "Locations")

if (${PLATFORM_IS_MACOS})
    add_library(${TARGET_NAME} INTERFACE)
    set(LOCATIONS_MIGR_PATH "${CMAKE_SOURCE_DIR}/src/Persistence/res/migr/Locations.csv")
    set_source_files_properties(
        ${LOCATIONS_MIGR_PATH}
        TARGET_DIRECTORY
            ${APP_NAME}
        PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources/migr)
    set_target_properties(
        ${TARGET_NAME}
        PROPERTIES
            INTERFACE_SOURCES "${LOCATIONS_MIGR_PATH}"
    )
else()
    add_custom_target(${TARGET_NAME} ALL)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_SOURCE_DIR}/src/Persistence/res/migr/Locations.csv
                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Resources/migr/Locations.csv
    )
endif()
