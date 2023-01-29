set(MODULE_NAME "GPXExport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/GpxExportPlugin.h src/GpxExportPlugin.cpp
        src/GpxExportSettings.h src/GpxExportSettings.cpp
        src/GpxExportOptionWidget.h src/GpxExportOptionWidget.cpp src/GpxExportOptionWidget.ui
        src/GpxExportPlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)
