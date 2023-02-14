set(MODULE_NAME "KmlExport")

qt_add_library(${MODULE_NAME} MODULE)
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
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)
