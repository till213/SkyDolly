set(GEOIDS "Geoids")

add_custom_target(${GEOIDS} ALL
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.pgm
        ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.pgm.aux.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.wld
)

add_custom_command(TARGET ${GEOIDS} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.pgm
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoids/egm2008-5.pgm
)
add_custom_command(TARGET ${GEOIDS} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.pgm.aux.xml
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoids/egm2008-5.pgm.aux.xml
)
add_custom_command(TARGET ${GEOIDS} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.wld
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoids/egm2008-5.wld
)
