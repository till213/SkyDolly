set(LIBRARY_NAME "PluginManager")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        PLUGINMANAGER_EXPORT
)
target_sources(${LIBRARY_NAME}
    PRIVATE
        include/PluginManager/PluginManagerLib.h
        include/PluginManager/Connect.h
        include/PluginManager/CsvConst.h
        include/PluginManager/SkyConnectIntf.h
        include/PluginManager/SettingsIntf.h
        include/PluginManager/PluginIntf.h
        include/PluginManager/ExportIntf.h
        include/PluginManager/ImportIntf.h
        include/PluginManager/SkyConnectManager.h src/SkyConnectManager.cpp
        include/PluginManager/AbstractSkyConnect.h src/AbstractSkyConnect.cpp
        include/PluginManager/PluginManager.h src/PluginManager.cpp
        include/PluginManager/Export.h src/Export.cpp
        include/PluginManager/PluginBase.h src/PluginBase.cpp
        include/PluginManager/FlightImportPluginBase.h src/FlightImportPluginBase.cpp
        include/PluginManager/FlightImportPluginBaseSettings.h src/FlightImportPluginBaseSettings.cpp
        include/PluginManager/BasicImportDialog.h src/BasicImportDialog.cpp src/BasicImportDialog.ui
        include/PluginManager/FlightExportPluginBase.h src/FlightExportPluginBase.cpp
        include/PluginManager/FlightExportPluginBaseSettings.h src/FlightExportPluginBaseSettings.cpp
        include/PluginManager/BasicExportDialog.h src/BasicExportDialog.cpp src/BasicExportDialog.ui
        include/PluginManager/ModuleIntf.h
        include/PluginManager/AbstractModule.h src/AbstractModule.cpp
        include/PluginManager/ModuleManager.h src/ModuleManager.cpp
        src/DefaultModuleImpl.h src/DefaultModuleImpl.cpp
        res/PluginManager.qrc
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        include/PluginManager
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Flight
        Sky::Kernel
        Sky::Model
        tsl::ordered_map
    PRIVATE
        Sky::Persistence
        Sky::Widget
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
