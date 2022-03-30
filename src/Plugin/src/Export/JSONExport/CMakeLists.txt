set(LIBRARY_NAME "JSONExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Plugin
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

set(DOCUMENTATION
    doc/README.md
    doc/Example.json
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/JSONExportPlugin.h src/JSONExportPlugin.cpp
        src/JSONExportSettings.h src/JSONExportSettings.cpp
        src/JSONExportPlugin.json
        ${DOCUMENTATION}
)

target_include_directories(${LIBRARY_NAME} PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
