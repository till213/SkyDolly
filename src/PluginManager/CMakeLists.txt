set(LIBRARY_NAME "PluginManager")

qt_add_library(${LIBRARY_NAME} SHARED)
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        PLUGINMANAGER_EXPORT
)
target_sources(${LIBRARY_NAME}
    PRIVATE
        include/PluginManager/PluginManagerLib.h
        include/PluginManager/Connect.h
        include/PluginManager/Csv.h
        include/PluginManager/SkyConnectIntf.h
        include/PluginManager/SettingsIntf.h
        include/PluginManager/PluginIntf.h
        include/PluginManager/FlightExportIntf.h
        include/PluginManager/FlightImportIntf.h
        include/PluginManager/SkyConnectManager.h src/SkyConnectManager.cpp
        include/PluginManager/AbstractSkyConnect.h src/AbstractSkyConnect.cpp
        include/PluginManager/PluginManager.h src/PluginManager.cpp
        include/PluginManager/Export.h src/Export.cpp
        include/PluginManager/PluginBase.h src/PluginBase.cpp
        include/PluginManager/FlightImportPluginBase.h src/FlightImportPluginBase.cpp
        include/PluginManager/FlightImportPluginBaseSettings.h src/FlightImportPluginBaseSettings.cpp
        include/PluginManager/BasicFlightImportDialog.h src/BasicFlightImportDialog.cpp src/BasicFlightImportDialog.ui
        include/PluginManager/FlightExportPluginBase.h src/FlightExportPluginBase.cpp
        include/PluginManager/FlightExportPluginBaseSettings.h src/FlightExportPluginBaseSettings.cpp
        include/PluginManager/BasicFlightExportDialog.h src/BasicFlightExportDialog.cpp src/BasicFlightExportDialog.ui
        include/PluginManager/LocationExportIntf.h
        include/PluginManager/LocationExportPluginBase.h src/LocationExportPluginBase.cpp
        include/PluginManager/LocationExportPluginBaseSettings.h src/LocationExportPluginBaseSettings.cpp
        include/PluginManager/BasicLocationExportDialog.h src/BasicLocationExportDialog.cpp src/BasicLocationExportDialog.ui
        include/PluginManager/LocationImportIntf.h
        include/PluginManager/LocationImportPluginBase.h src/LocationImportPluginBase.cpp
        include/PluginManager/LocationImportPluginBaseSettings.h src/LocationImportPluginBaseSettings.cpp
        include/PluginManager/BasicLocationImportDialog.h src/BasicLocationImportDialog.cpp src/BasicLocationImportDialog.ui
        include/PluginManager/ModuleIntf.h
        include/PluginManager/AbstractModule.h src/AbstractModule.cpp
        include/PluginManager/ModuleManager.h src/ModuleManager.cpp
        src/DefaultModuleImpl.h src/DefaultModuleImpl.cpp
        src/Module/DefaultModuleSettings.h src/Module/DefaultModuleSettings.cpp
        include/PluginManager/Module/ModuleBaseSettings.h src/Module/ModuleBaseSettings.cpp
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
        Qt6::Widgets
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
