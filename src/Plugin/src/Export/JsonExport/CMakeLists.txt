set(LIBRARY_NAME "JSONExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} MODULE)
else()
    add_library(${LIBRARY_NAME} MODULE)
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin
)

set(DOCUMENTATION
    doc/README.md
    doc/Example.json
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/JsonExportPlugin.h src/JsonExportPlugin.cpp
        src/JsonExportSettings.h src/JsonExportSettings.cpp
        src/JsonExportPlugin.json
        ${DOCUMENTATION}
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
