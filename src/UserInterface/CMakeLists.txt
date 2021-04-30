add_definitions(-DUSERINTERFACE_EXPORT)

qt5_add_resources(USERINTERFACE_SOURCES res/SkyDolly.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(UserInterface SHARED
        ${USERINTERFACE_SOURCES}
    )
else()
    add_library(UserInterface SHARED
        ${USERINTERFACE_SOURCES}
    )
endif()

set(USERINTERFACE_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Persistence SkyConnect
)
if (APPLE)
    list(APPEND USERINTERFACE_LIBS -lc++)
endif()

target_sources(UserInterface
    PRIVATE
        src/Widgets/ActionButton.cpp
        src/Widgets/ActionButton.h
        src/UserInterfaceLib.h
        src/MainWindow.cpp
        src/MainWindow.h
        src/MainWindow.ui
        src/Unit.cpp
        src/Unit.h
        src/Dialogs/AboutDialog.cpp
        src/Dialogs/AboutDialog.h
        src/Dialogs/AboutDialog.ui
        src/Dialogs/AboutLibraryDialog.cpp
        src/Dialogs/AboutLibraryDialog.h
        src/Dialogs/AboutLibraryDialog.ui
        src/Dialogs/SimulationVariablesDialog.cpp
        src/Dialogs/SimulationVariablesDialog.h
        src/Dialogs/SimulationVariablesDialog.ui
        src/Dialogs/SettingsDialog.cpp
        src/Dialogs/SettingsDialog.h
        src/Dialogs/SettingsDialog.ui
        src/Dialogs/StatisticsDialog.cpp
        src/Dialogs/StatisticsDialog.h
        src/Dialogs/StatisticsDialog.ui
        src/Dialogs/ScenarioDialog.cpp
        src/Dialogs/ScenarioDialog.h
        src/Dialogs/ScenarioDialog.ui
        src/Widgets/AircraftWidget.cpp
        src/Widgets/AircraftWidget.h
        src/Widgets/AircraftWidget.ui
        src/Widgets/EngineWidget.cpp
        src/Widgets/EngineWidget.h
        src/Widgets/EngineWidget.ui
        src/Widgets/PrimaryFlightControlWidget.cpp
        src/Widgets/PrimaryFlightControlWidget.h
        src/Widgets/PrimaryFlightControlWidget.ui
        src/Widgets/SecondaryFlightControlWidget.cpp
        src/Widgets/SecondaryFlightControlWidget.h
        src/Widgets/SecondaryFlightControlWidget.ui
        src/Widgets/AircraftHandleWidget.cpp
        src/Widgets/AircraftHandleWidget.h
        src/Widgets/AircraftHandleWidget.ui
        src/Widgets/AircraftTypeWidget.cpp
        src/Widgets/AircraftTypeWidget.h
        src/Widgets/AircraftTypeWidget.ui
        src/Widgets/FlightConditionWidget.cpp
        src/Widgets/FlightConditionWidget.h
        src/Widgets/FlightConditionWidget.ui
        src/Widgets/LightWidget.cpp
        src/Widgets/LightWidget.h
        src/Widgets/LightWidget.ui
        src/Widgets/ScenarioWidget.cpp
        src/Widgets/ScenarioWidget.h
        src/Widgets/ScenarioWidget.ui
)

target_link_libraries(UserInterface PRIVATE ${USERINTERFACE_LIBS})
set_target_properties(UserInterface PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(UserInterface PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
