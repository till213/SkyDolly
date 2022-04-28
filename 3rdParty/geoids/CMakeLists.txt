set(TARGET_NAME "Geoids")

include(FetchEarthGravityModel)

add_custom_target(${TARGET_NAME} ALL)

# Option: SD_FETCH_EGM (default: OFF)
if (egm_POPULATED)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${egm_SOURCE_DIR}/egm2008-5.pgm
                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoids/egm2008-5.pgm
    )
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${egm_SOURCE_DIR}/egm2008-5.pgm.aux.xml
                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoids/egm2008-5.pgm.aux.xml
    )
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${egm_SOURCE_DIR}/egm2008-5.wld
                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoids/egm2008-5.wld
    )
    message(STATUS "[SD_FETCH_EGM=ON] Downloaded earth gravity model egm2008-5.pgm.")
else()
    message(STATUS "[SD_FETCH_EGM=OFF] No earth gravity model downloaded: enable option SD_FETCH_EGM and/or check internet connection.")
endif()
