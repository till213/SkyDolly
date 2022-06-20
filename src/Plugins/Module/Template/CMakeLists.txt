set(MODULE_NAME "Template")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/TemplatePlugin.h src/TemplatePlugin.cpp
        src/TemplateWidget.h src/TemplateWidget.cpp src/TemplateWidget.ui
        src/TemplatePlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Persistence
        Sky::PluginManager
        Sky::Widget
)
