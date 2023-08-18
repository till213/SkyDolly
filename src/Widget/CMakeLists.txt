set(LIBRARY_NAME "Widget")

qt_add_library(${LIBRARY_NAME} SHARED)
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        WIDGET_EXPORT
)
target_sources(${LIBRARY_NAME}
    PRIVATE
        include/Widget/WidgetLib.h
        include/Widget/ActiveButton.h src/ActiveButton.cpp
        include/Widget/ActionButton.h src/ActionButton.cpp
        include/Widget/ActionRadioButton.h src/ActionRadioButton.cpp
        include/Widget/ActionCheckBox.h src/ActionCheckBox.cpp
        include/Widget/FocusPlainTextEdit.h src/FocusPlainTextEdit.cpp
        include/Widget/AircraftSelectionComboBox.h src/AircraftSelectionComboBox.cpp
        include/Widget/Platform.h src/Platform.cpp
        include/Widget/TableDateItem.h src/TableDateItem.cpp
        include/Widget/TableTimeItem.h src/TableTimeItem.cpp
        include/Widget/HyperlinkTextEdit.h src/HyperlinkTextEdit.cpp
        include/Widget/EnumerationComboBox.h src/EnumerationComboBox.cpp
        include/Widget/EnumerationWidgetItem.h src/EnumerationWidgetItem.cpp
        include/Widget/UnitWidgetItem.h src/UnitWidgetItem.cpp
        include/Widget/LinkedOptionGroup.h src/LinkedOptionGroup.cpp
        include/Widget/RecentFileMenu.h src/RecentFileMenu.cpp
)
if(${PLATFORM_IS_WINDOWS})
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/Platform_Windows.cpp
    )
elseif(${PLATFORM_IS_MACOS})
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/Platform_macOS.cpp
    )
else()
    target_sources(${LIBRARY_NAME}
        PRIVATE
            src/Platform_Linux.cpp
    )
endif()
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        include/Widget
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt6::Widgets
    PRIVATE
        Sky::Kernel
        Sky::Model
        Sky::Persistence
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
