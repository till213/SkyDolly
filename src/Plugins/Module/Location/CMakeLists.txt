set(MODULE_NAME "Location")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        src/LocationPlugin.h src/LocationPlugin.cpp
        src/LocationSettings.h src/LocationSettings.cpp
        src/LocationWidget.h src/LocationWidget.cpp src/LocationWidget.ui
        src/PositionWidgetItem.h src/PositionWidgetItem.cpp
        src/EnumerationItemDelegate.h src/EnumerationItemDelegate.cpp
        src/DateTimeItemDelegate.h src/DateTimeItemDelegate.cpp
        src/LocationPlugin.json
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
