set(LIBRARY_NAME "Module")

add_definitions(-DMODULE_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Persistence SkyConnect Plugin Widget tsl::ordered_map
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/ModuleLib.h
        src/Module.h
        src/ModuleIntf.h
        src/AbstractModuleWidget.cpp
        src/AbstractModuleWidget.h
        src/ModuleManager.cpp
        src/ModuleManager.h
        src/Logbook/LogbookWidget.cpp
        src/Logbook/LogbookWidget.h
        src/Logbook/LogbookWidget.ui
        src/Formation/FormationWidget.cpp
        src/Formation/FormationWidget.h
        src/Formation/FormationWidget.ui
)

target_include_directories(${LIBRARY_NAME}
    PRIVATE
        ../Widget/src/
)
target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
