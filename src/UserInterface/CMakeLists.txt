set(LIBRARY_NAME "UserInterface")

add_definitions(-DUSERINTERFACE_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Persistence Widget SkyConnect Plugin Module
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        res/SkyDolly.qrc
        src/UserInterfaceLib.h
        src/MainWindow.cpp
        src/MainWindow.h
        src/MainWindow.ui
        src/Dialog/AboutDialog.cpp
        src/Dialog/AboutDialog.h
        src/Dialog/AboutDialog.ui
        src/Dialog/LogbookSettingsDialog.cpp
        src/Dialog/LogbookSettingsDialog.h
        src/Dialog/LogbookSettingsDialog.ui
        src/Dialog/SimulationVariablesDialog.cpp
        src/Dialog/SimulationVariablesDialog.h
        src/Dialog/SimulationVariablesDialog.ui
        src/Dialog/SettingsDialog.cpp
        src/Dialog/SettingsDialog.h
        src/Dialog/SettingsDialog.ui
        src/Dialog/StatisticsDialog.cpp
        src/Dialog/StatisticsDialog.h
        src/Dialog/StatisticsDialog.ui
        src/Dialog/FlightDialog.cpp
        src/Dialog/FlightDialog.h
        src/Dialog/FlightDialog.ui
        src/Dialog/LogbookBackupDialog.cpp
        src/Dialog/LogbookBackupDialog.h
        src/Dialog/LogbookBackupDialog.ui
        src/Widget/FlightDescriptionWidget.cpp
        src/Widget/FlightDescriptionWidget.h
        src/Widget/FlightDescriptionWidget.ui
        src/Widget/AbstractSimulationVariableWidget.cpp
        src/Widget/AbstractSimulationVariableWidget.h
        src/Widget/AircraftWidget.cpp
        src/Widget/AircraftWidget.h
        src/Widget/AircraftWidget.ui
        src/Widget/EngineWidget.cpp
        src/Widget/EngineWidget.h
        src/Widget/EngineWidget.ui
        src/Widget/PrimaryFlightControlWidget.cpp
        src/Widget/PrimaryFlightControlWidget.h
        src/Widget/PrimaryFlightControlWidget.ui
        src/Widget/SecondaryFlightControlWidget.cpp
        src/Widget/SecondaryFlightControlWidget.h
        src/Widget/SecondaryFlightControlWidget.ui
        src/Widget/AircraftHandleWidget.cpp
        src/Widget/AircraftHandleWidget.h
        src/Widget/AircraftHandleWidget.ui
        src/Widget/LightWidget.cpp
        src/Widget/LightWidget.h
        src/Widget/LightWidget.ui
        src/Widget/AircraftTypeWidget.cpp
        src/Widget/AircraftTypeWidget.h
        src/Widget/AircraftTypeWidget.ui
        src/Widget/FlightConditionWidget.cpp
        src/Widget/FlightConditionWidget.h
        src/Widget/FlightConditionWidget.ui
        src/Widget/FlightPlanWidget.cpp
        src/Widget/FlightPlanWidget.h
        src/Widget/FlightPlanWidget.ui
        src/Widget/WaypointWidget.cpp
        src/Widget/WaypointWidget.h
        src/Widget/WaypointWidget.ui
)

include_directories(UserInterface PRIVATE ../Widget/src/)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
set_target_properties(${LIBRARY_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${LIBRARY_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
