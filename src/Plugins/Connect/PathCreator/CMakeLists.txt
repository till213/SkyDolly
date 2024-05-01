set(MODULE_NAME "PathCreator")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/PathCreatorPlugin.h src/PathCreatorPlugin.cpp
        src/PathCreatorSettings.h src/PathCreatorSettings.cpp
        src/PathCreatorPlugin.json
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)
