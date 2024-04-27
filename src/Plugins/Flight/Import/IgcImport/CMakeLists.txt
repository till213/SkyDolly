set(MODULE_NAME "IgcImport")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/IgcImportPlugin.h src/IgcImportPlugin.cpp
        src/IgcParser.h src/IgcParser.cpp
        src/IgcImportSettings.h src/IgcImportSettings.cpp
        src/IgcImportOptionWidget.h src/IgcImportOptionWidget.cpp src/IgcImportOptionWidget.ui
        src/IgcImportPlugin.json
        doc/README.md
        doc/2022-01-31-XYY-EXAMPLE-01.IGC
        doc/2022-01-31-XYY-EXAMPLE-02.IGC
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Widget
        Sky::PluginManager
)
