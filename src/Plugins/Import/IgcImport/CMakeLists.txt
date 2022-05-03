set(MODULE_NAME "IGCImport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
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
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Widget
        Sky::PluginManager
)
