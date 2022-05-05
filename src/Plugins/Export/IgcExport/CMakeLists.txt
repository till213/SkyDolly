set(MODULE_NAME "IGCExport")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/IgcExportPlugin.h src/IgcExportPlugin.cpp
        src/IgcExportSettings.h src/IgcExportSettings.cpp
        src/IgcExportOptionWidget.h src/IgcExportOptionWidget.cpp src/IgcExportOptionWidget.ui
        src/IgcExportPlugin.json
        doc/README.md
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)
