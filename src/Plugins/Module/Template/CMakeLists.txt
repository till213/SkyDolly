set(MODULE_NAME "Template")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/TemplatePlugin.h src/TemplatePlugin.cpp
        src/TemplateSettings.h src/TemplateSettings.cpp
        src/TemplateWidget.h src/TemplateWidget.cpp src/TemplateWidget.ui
        src/TemplatePlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Persistence
        Sky::PluginManager
        Sky::Widget
)
