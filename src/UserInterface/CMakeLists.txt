set(LIBRARY_NAME "UserInterface")

add_definitions(-DUSERINTERFACE_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets
    Sky::Kernel
    Sky::Model
    Sky::Persistence
    Sky::Widget
    Sky::PluginManager
    Sky::Module
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        res/SkyDolly.qrc
        src/UserInterfaceLib.h
        src/MainWindow.h src/MainWindow.cpp src/MainWindow.ui
        src/Dialog/AboutDialog.h src/Dialog/AboutDialog.cpp src/Dialog/AboutDialog.ui
        src/Dialog/LogbookSettingsDialog.h src/Dialog/LogbookSettingsDialog.cpp src/Dialog/LogbookSettingsDialog.ui
        src/Dialog/SimulationVariablesDialog.h src/Dialog/SimulationVariablesDialog.cpp src/Dialog/SimulationVariablesDialog.ui
        src/Dialog/SettingsDialog.h src/Dialog/SettingsDialog.cpp src/Dialog/SettingsDialog.ui
        src/Dialog/StatisticsDialog.h src/Dialog/StatisticsDialog.cpp src/Dialog/StatisticsDialog.ui
        src/Dialog/FlightDialog.h src/Dialog/FlightDialog.cpp src/Dialog/FlightDialog.ui
        src/Dialog/LogbookBackupDialog.h src/Dialog/LogbookBackupDialog.cpp src/Dialog/LogbookBackupDialog.ui
        src/Widget/FlightDescriptionWidget.h src/Widget/FlightDescriptionWidget.cpp src/Widget/FlightDescriptionWidget.ui
        src/Widget/AbstractSimulationVariableWidget.h src/Widget/AbstractSimulationVariableWidget.cpp
        src/Widget/AircraftWidget.h src/Widget/AircraftWidget.cpp src/Widget/AircraftWidget.ui
        src/Widget/EngineWidget.h src/Widget/EngineWidget.cpp src/Widget/EngineWidget.ui
        src/Widget/PrimaryFlightControlWidget.h src/Widget/PrimaryFlightControlWidget.cpp src/Widget/PrimaryFlightControlWidget.ui
        src/Widget/SecondaryFlightControlWidget.h src/Widget/SecondaryFlightControlWidget.cpp src/Widget/SecondaryFlightControlWidget.ui
        src/Widget/AircraftHandleWidget.h src/Widget/AircraftHandleWidget.cpp src/Widget/AircraftHandleWidget.ui
        src/Widget/LightWidget.h src/Widget/LightWidget.cpp src/Widget/LightWidget.ui
        src/Widget/AircraftInfoWidget.h src/Widget/AircraftInfoWidget.cpp src/Widget/AircraftInfoWidget.ui
        src/Widget/FlightConditionWidget.h src/Widget/FlightConditionWidget.cpp src/Widget/FlightConditionWidget.ui
        src/Widget/FlightPlanWidget.h src/Widget/FlightPlanWidget.cpp src/Widget/FlightPlanWidget.ui
        src/Widget/WaypointWidget.h src/Widget/WaypointWidget.cpp src/Widget/WaypointWidget.ui
)

target_include_directories(${LIBRARY_NAME} PRIVATE ../Widget/src/)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
