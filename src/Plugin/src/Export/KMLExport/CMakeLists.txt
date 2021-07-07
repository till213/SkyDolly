set(PLUGIN_NAME "KMLExportPlugin")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${PLUGIN_NAME} SHARED "")
else()
    add_library(${PLUGIN_NAME} SHARED "")
endif()

target_sources(${PLUGIN_NAME}
    PRIVATE
        src/KMLExportPlugin.cpp
        src/KMLExportPlugin.h
        src/KMLExportPlugin.json
)

set(PLUGIN_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model
)
if (APPLE)
    list(APPEND PLUGIN_LIBS -lc++)
endif()

target_link_libraries(${PLUGIN_NAME} PRIVATE ${PLUGIN_LIBS})

target_include_directories(${PLUGIN_NAME} PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

