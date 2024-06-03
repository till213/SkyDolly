set(MODULE_NAME "SdLogLocationImport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/SdLogLocationImportPlugin.h src/SdLogLocationImportPlugin.cpp
        src/SdLogLocationImportSettings.h src/SdLogLocationImportSettings.cpp
        src/SdLogLocationImportPlugin.json
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
