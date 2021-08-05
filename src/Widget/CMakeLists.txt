set(LIBRARY_NAME "Widget")

add_definitions(-DWIDGET_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Persistence
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/WidgetLib.h
        src/ActiveButton.cpp
        src/ActiveButton.h
        src/ActionButton.cpp
        src/ActionButton.h
        src/ActionRadioButton.h
        src/ActionRadioButton.cpp
        src/ActionCheckBox.h
        src/ActionCheckBox.cpp
        src/FocusPlainTextEdit.h
        src/FocusPlainTextEdit.cpp
        src/AircraftSelectionComboBox.cpp
        src/AircraftSelectionComboBox.h
        src/BackupPeriodComboBox.cpp
        src/BackupPeriodComboBox.h
        src/Platform.cpp
        src/Platform.h
        src/TableDateItem.cpp
        src/TableDateItem.h
        src/TableTimeItem.cpp
        src/TableTimeItem.h
)

target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
set_target_properties(${LIBRARY_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${LIBRARY_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
