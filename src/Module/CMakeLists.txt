set(LIBRARY_NAME "Module")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        MODULE_EXPORT
)
target_sources(${LIBRARY_NAME}
    PRIVATE
        include/Module/ModuleLib.h
        include/Module/Module.h
        include/Module/ModuleIntf.h
        include/Module/AbstractModuleWidget.h src/AbstractModuleWidget.cpp
        include/Module/ModuleManager.h src/ModuleManager.cpp
        src/Logbook/LogbookWidget.h src/Logbook/LogbookWidget.cpp src/Logbook/LogbookWidget.ui
        src/Formation/FormationWidget.h src/Formation/FormationWidget.cpp src/Formation/FormationWidget.ui
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        include/Module
        ../Widget/src/
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE
        Sky::Kernel
        Sky::Model
        Sky::Persistence
        Sky::PluginManager
        Sky::Widget
        tsl::ordered_map
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
