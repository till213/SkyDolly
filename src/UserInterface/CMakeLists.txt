add_definitions(-DUSERINTERFACE_EXPORT)

qt5_add_resources(PROJECT_SOURCES res/SkyDolly.qrc)

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
    Qt${QT_VERSION_MAJOR}::Widgets Kernel SkyConnect
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
        src/Dialogs/AboutDialog.cpp
        src/Dialogs/AboutDialog.h
        src/Dialogs/AboutDialog.ui
        src/Dialogs/SimulationVariablesDialog.cpp
        src/Dialogs/SimulationVariablesDialog.h
        src/Dialogs/SimulationVariablesDialog.ui
        src/Dialogs/SettingsDialog.cpp
        src/Dialogs/SettingsDialog.h
        src/Dialogs/SettingsDialog.ui
)

target_link_libraries(UserInterface PRIVATE ${USERINTERFACE_LIBS})
set_target_properties(UserInterface PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(UserInterface PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
