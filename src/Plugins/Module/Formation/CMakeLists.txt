set(MODULE_NAME "Formation")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/Formation.h src/Formation.cpp
        src/FormationPlugin.h src/FormationPlugin.cpp
        src/FormationWidget.h src/FormationWidget.cpp src/FormationWidget.ui
        src/FormationPlugin.json
        res/FormationPlugin.qrc
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
