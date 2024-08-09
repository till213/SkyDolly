set(MODULE_NAME "KmlLocationImport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/KmlLocationImportPlugin.h src/KmlLocationImportPlugin.cpp
        src/KmlLocationImportSettings.h src/KmlLocationImportSettings.cpp
        src/KmlLocationImportPlugin.json
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
