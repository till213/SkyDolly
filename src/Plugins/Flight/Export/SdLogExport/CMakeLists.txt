set(MODULE_NAME "SdLogExport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/SdLogExportPlugin.h src/SdLogExportPlugin.cpp
        src/SdLogExportSettings.h src/SdLogExportSettings.cpp
        src/SdLogExportPlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)
