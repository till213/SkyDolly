set(MODULE_NAME "SDLogImport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/SdLogImportPlugin.h src/SdLogImportPlugin.cpp
        src/SdLogImportSettings.h src/SdLogImportSettings.cpp
        src/SdLogImportPlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Persistence
        Sky::PluginManager
)
