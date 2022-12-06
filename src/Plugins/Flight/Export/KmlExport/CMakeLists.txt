set(MODULE_NAME "KMLExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/KmlExportPlugin.h src/KmlExportPlugin.cpp
        src/KmlExportSettings.h src/KmlExportSettings.cpp
        src/KmlExportOptionWidget.h src/KmlExportOptionWidget.cpp src/KmlExportOptionWidget.ui
        src/KmlStyleExport.h src/KmlStyleExport.cpp
        src/KmlExportPlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)
