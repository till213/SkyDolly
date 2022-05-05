set(MODULE_NAME "GPXImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/GpxImportPlugin.h src/GpxImportPlugin.cpp
        src/GpxImportSettings.h src/GpxImportSettings.cpp
        src/GpxImportOptionWidget.h src/GpxImportOptionWidget.cpp src/GpxImportOptionWidget.ui
        src/Gpx.h
        src/GpxParser.h src/GpxParser.cpp
        src/GpxImportPlugin.json
        doc/README.md
        doc/Sample.gpx
        doc/Nile.gpx
        doc/Nile-NoElevation-NoTimestamp.gpx
        doc/LittleNavmap.gpx
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Widget
        Sky::PluginManager
)
