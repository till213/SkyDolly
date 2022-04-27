include(FetchContent)

set(CMAKE_TLS_VERIFY true)

function(download_file url hash)
    FetchContent_Declare(download_${hash}
        URL ${url}
        URL_HASH SHA256=${hash}
        DOWNLOAD_NO_EXTRACT true
    )
    FetchContent_MakeAvailable(download_${hash})
endfunction(download_file)

set(TARGET_NAME "Geoids")

add_custom_target(${TARGET_NAME} ALL
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.pgm
        ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.pgm.aux.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.wld
)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.pgm
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoids/egm2008-5.pgm
)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.pgm.aux.xml
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoids/egm2008-5.pgm.aux.xml
)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/egm2008-5.wld
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geoids/egm2008-5.wld
)

# Will be placed at: ./_deps/download_12794390cce7d0682ffc783c785e4282305684431b30b29ed75c224da24035b4-src/
#download_file(
#  https://upload.wikimedia.org/wikipedia/commons/9/91/Octicons-mark-github.svg
#  12794390cce7d0682ffc783c785e4282305684431b30b29ed75c224da24035b4
#)
