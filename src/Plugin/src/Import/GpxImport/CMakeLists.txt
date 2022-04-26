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
        src/GpxImportPlugin.h src/GpxImportPlugin.cpp
        src/GpxImportSettings.h src/GpxImportSettings.cpp
        src/GpxImportOptionWidget.h src/GpxImportOptionWidget.cpp src/GpxImportOptionWidget.ui
        src/Gpx.h
        src/GpxParser.h src/GpxParser.cpp
        src/GpxImportPlugin.json
        ${DOCUMENTATION}
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
