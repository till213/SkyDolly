add_definitions(-DMODULE_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(Module SHARED
        ${MODULE_SOURCES}
    )
else()
    add_library(Module SHARED
        ${MODULE_SOURCES}
    )
endif()

set(MODULE_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Persistence SkyConnect
)
if (APPLE)
    list(APPEND MODULE_LIBS -lc++)
endif()

target_sources(Module
    PRIVATE
        src/ModuleLib.h
        src/ModuleIntf.h
        src/Logbook/LogbookWidget.cpp
        src/Logbook/LogbookWidget.h
        src/Logbook/LogbookWidget.ui
)

include_directories(Module PRIVATE src/Widgets)
target_link_libraries(Module PRIVATE ${MODULE_LIBS})
set_target_properties(Module PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Module PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
