set(TARGET_NAME "Geoids")

include(FetchEarthGravityModel)

add_custom_target(${TARGET_NAME} ALL)

# Option: SD_FETCH_EGM (default: OFF)
if (egm_POPULATED)

    message(STATUS "[SD_FETCH_EGM=ON] Downloaded earth gravity model egm2008-5.pgm.")

    if (${PLATFORM_IS_MACOS})
        set(EGM_FILE_PATH
            "${egm_SOURCE_DIR}/egm2008-5.pgm"
            "${egm_SOURCE_DIR}/egm2008-5.pgm.aux.xml"
            "${egm_SOURCE_DIR}/egm2008-5.wld"
        )
        set_source_files_properties(
            ${EGM_FILE_PATH}
            TARGET_DIRECTORY
                SkyDolly
            PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources/geoids)

        set_target_properties(
            ${TARGET_NAME}
            PROPERTIES
                INTERFACE_SOURCES "${EGM_FILE_PATH}"
        )
    else()
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${egm_SOURCE_DIR}/egm2008-5.pgm
                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Resources/geoids/egm2008-5.pgm
        )
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${egm_SOURCE_DIR}/egm2008-5.pgm.aux.xml
                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Resources/geoids/egm2008-5.pgm.aux.xml
        )
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${egm_SOURCE_DIR}/egm2008-5.wld
                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Resources/geoids/egm2008-5.wld
        )
    endif()

else()
    message(STATUS "[SD_FETCH_EGM=OFF] No earth gravity model downloaded: enable option SD_FETCH_EGM and/or check internet connection.")
endif()
