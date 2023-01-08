set(MODULE_NAME "Logbook")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/LogbookPlugin.h src/LogbookPlugin.cpp
        src/LogbookWidget.h src/LogbookWidget.cpp src/LogbookWidget.ui
        src/LogbookPlugin.json
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
