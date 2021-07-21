set(PLUGIN_NAME "CSVExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${PLUGIN_NAME} SHARED
        ${PLUGIN_SOURCES}
    )
else()
    add_library(${PLUGIN_NAME} SHARED
        ${PLUGIN_SOURCES}
    )
endif()

target_sources(CSVExport
    PRIVATE
        src/CSVExportPlugin.cpp
        src/CSVExportPlugin.h
        src/CSVExportPlugin.json
)

set(PLUGIN_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin
)
if (${PLATFORM_IS_MACOS})
    list(APPEND PLUGIN_DEPENDENCIES -lc++)
endif()

target_link_libraries(${PLUGIN_NAME} PRIVATE ${PLUGIN_DEPENDENCIES})

target_include_directories(${PLUGIN_NAME} PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)


