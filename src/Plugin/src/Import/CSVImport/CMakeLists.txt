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
)

set(PLUGIN_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin Persistence
)
if (${PLATFORM_IS_MACOS})
    list(APPEND PLUGIN_LIBS -lc++)
endif()

target_link_libraries(${PLUGIN_NAME} PRIVATE ${PLUGIN_LIBS})

target_include_directories(${PLUGIN_NAME} PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)
