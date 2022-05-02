set(LIBRARY_NAME "PluginManager")

add_definitions(-DPLUGINMANAGER_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/PluginManagerLib.h
        src/SkyConnectManager.cpp
        src/SkyConnectManager.h
        src/SkyConnectIntf.h
        src/Connect.h
        src/AbstractSkyConnect.cpp
        src/AbstractSkyConnect.h
        src/PluginManager.h src/PluginManager.cpp
        src/PluginIntf.h
        src/ExportIntf.h
        src/ImportIntf.h      
        src/Export.h src/Export.cpp
        src/PluginBase.h src/PluginBase.cpp
        src/ImportPluginBase.h src/ImportPluginBase.cpp
        src/ImportPluginBaseSettings.h src/ImportPluginBaseSettings.cpp
        src/BasicImportDialog.h src/BasicImportDialog.cpp src/BasicImportDialog.ui
        src/ExportPluginBase.h src/ExportPluginBase.cpp
        src/ExportPluginBaseSettings.h src/ExportPluginBaseSettings.cpp        
        src/BasicExportDialog.h src/BasicExportDialog.cpp src/BasicExportDialog.ui
        src/SettingsIntf.h
        src/CsvConst.h
)

target_include_directories(${LIBRARY_NAME}
    PRIVATE
        ../Widget/src/
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Flight
    PRIVATE
        Sky::Kernel
        Sky::Model

        Sky::Persistence
        Sky::Widget
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)