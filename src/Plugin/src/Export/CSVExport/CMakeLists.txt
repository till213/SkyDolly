if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(CSVExport SHARED "")
else()
    add_library(CSVExport SHARED "")
endif()

target_sources(CSVExport
    PRIVATE
        src/CSVExportPlugin.cpp
        src/CSVExportPlugin.h
        src/CSVExportPlugin.json
)

set(CSV_EXPORT_PLUGIN_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin
)
if (APPLE)
    list(APPEND CSV_EXPORT_PLUGIN_LIBS -lc++)
endif()

target_link_libraries(CSVExport PRIVATE ${CSV_EXPORT_PLUGIN_LIBS})

target_include_directories(CSVExport PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

