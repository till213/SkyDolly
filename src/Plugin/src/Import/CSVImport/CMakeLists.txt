set(PLUGIN_NAME "CSVImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${PLUGIN_NAME} SHARED
        ${PLUGIN_SOURCES}
    )
else()
    add_library(${PLUGIN_NAME} SHARED
        ${PLUGIN_SOURCES}
    )
endif()

target_sources(CSVImport
    PRIVATE
        src/CSVImportPlugin.cpp
        src/CSVImportPlugin.h
        src/CSVImportPlugin.json
        src/CSVImportDialog.cpp
        src/CSVImportDialog.h
        src/CSVImportDialog.ui
)

set(PLUGIN_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Persistence Widget Plugin
)
if (${PLATFORM_IS_MACOS})
    list(APPEND PLUGIN_DEPENDENCIES -lc++)
endif()

target_link_libraries(${PLUGIN_NAME} PRIVATE ${PLUGIN_DEPENDENCIES})

include_directories(${PLUGIN_NAME} PRIVATE ../../../../Widget/src/)
target_include_directories(${PLUGIN_NAME} PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)
