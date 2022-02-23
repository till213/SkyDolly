set(LIBRARY_NAME "GPXImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Flight Persistence Plugin Widget
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

set(DOCUMENTATION
    doc/README.md
    doc/Sample.gpx
    doc/Nile.gpx
    doc/Nile-NoElevation-NoTimestamp.gpx
    doc/LittleNavmap.gpx
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/GPXImportPlugin.h src/GPXImportPlugin.cpp
        src/GPXImportSettings.h src/GPXImportSettings.cpp
        src/GPXImportOptionWidget.h src/GPXImportOptionWidget.cpp src/GPXImportOptionWidget.ui
        src/GPX.h
        src/GPXParser.h src/GPXParser.cpp
        src/GPXImportPlugin.json
        ${DOCUMENTATION}
)

include_directories(${PLUGIN_NAME} PRIVATE ../../../../Widget/src/)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
