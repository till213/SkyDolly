if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(CSVImport SHARED "")
else()
    add_library(CSVImport SHARED "")
endif()

target_sources(CSVImport
    PRIVATE
        src/CSVImportPlugin.cpp
        src/CSVImportPlugin.h
        src/CSVImportPlugin.json
)

set(CSV_IMPORT_PLUGIN_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin Persistence
)
if (APPLE)
    list(APPEND PLUGIN_LIBS -lc++)
endif()

target_link_libraries(CSVImport PRIVATE ${CSV_IMPORT_PLUGIN_LIBS})

target_include_directories(CSVImport PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)
