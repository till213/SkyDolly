set(MODULE_NAME "JsonExport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/JsonExportPlugin.h src/JsonExportPlugin.cpp
        src/JsonExportSettings.h src/JsonExportSettings.cpp
        src/JsonExportPlugin.json
        doc/README.md
        doc/Example.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)
