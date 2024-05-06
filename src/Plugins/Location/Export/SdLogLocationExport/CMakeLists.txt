set(MODULE_NAME "SdLogLocationExport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/SdLogLocationExportPlugin.h src/SdLogLocationExportPlugin.cpp
        src/SdLogLocationExportSettings.h src/SdLogLocationExportSettings.cpp
        src/SdLogLocationExportPlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Persistence
        Sky::Widget
        Sky::PluginManager
)
