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
        include/PluginManager/Csv.h
        include/PluginManager/PluginIntf.h
        include/PluginManager/DialogPluginIntf.h
        include/PluginManager/PluginWithOptionWidgetIntf.h
        include/PluginManager/SkyConnectManager.h src/SkyConnectManager.cpp
        include/PluginManager/PluginManager.h src/PluginManager.cpp
        include/PluginManager/Export.h src/Export.cpp
        include/PluginManager/PluginBase.h src/PluginBase.cpp
        include/PluginManager/DialogPluginBase.h src/DialogPluginBase.cpp
        # SkyConnect pugins
        include/PluginManager/Connect/Connect.h
        include/PluginManager/Connect/SkyConnectIntf.h
        include/PluginManager/Connect/AbstractSkyConnect.h src/Connect/AbstractSkyConnect.cpp
        include/PluginManager/Connect/ConnectPluginBaseSettings.h src/Connect/ConnectPluginBaseSettings.cpp
        # Flight import & export plugins
        include/PluginManager/Flight/FlightExportIntf.h
        include/PluginManager/Flight/FlightImportIntf.h
        include/PluginManager/Flight/FlightImportPluginBase.h src/Flight/FlightImportPluginBase.cpp
        include/PluginManager/Flight/FlightImportPluginBaseSettings.h src/Flight/FlightImportPluginBaseSettings.cpp
        include/PluginManager/Flight/BasicFlightImportDialog.h src/Flight/BasicFlightImportDialog.cpp src/Flight/BasicFlightImportDialog.ui
        include/PluginManager/Flight/FlightExportPluginBase.h src/Flight/FlightExportPluginBase.cpp
        include/PluginManager/Flight/FlightExportPluginBaseSettings.h src/Flight/FlightExportPluginBaseSettings.cpp
        include/PluginManager/Flight/BasicFlightExportDialog.h src/Flight/BasicFlightExportDialog.cpp src/Flight/BasicFlightExportDialog.ui
        # Location import & export plugins
        include/PluginManager/Location/LocationExportIntf.h
        include/PluginManager/Location/LocationExportPluginBase.h src/Location/LocationExportPluginBase.cpp
        include/PluginManager/Location/LocationExportPluginBaseSettings.h src/Location/LocationExportPluginBaseSettings.cpp
        include/PluginManager/Location/BasicLocationExportDialog.h src/Location/BasicLocationExportDialog.cpp src/Location/BasicLocationExportDialog.ui
        include/PluginManager/Location/LocationImportIntf.h
        include/PluginManager/Location/LocationImportPluginBase.h src/Location/LocationImportPluginBase.cpp
        include/PluginManager/Location/LocationImportPluginBaseSettings.h src/Location/LocationImportPluginBaseSettings.cpp
        include/PluginManager/Location/BasicLocationImportDialog.h src/Location/BasicLocationImportDialog.cpp src/Location/BasicLocationImportDialog.ui
        # Modules
        include/PluginManager/ModuleManager.h src/ModuleManager.cpp
        include/PluginManager/Module/ModuleIntf.h
        include/PluginManager/Module/AbstractModule.h src/Module/AbstractModule.cpp        
        src/Module/DefaultModuleImpl.h src/Module/DefaultModuleImpl.cpp
        src/Module/DefaultModuleSettings.h src/Module/DefaultModuleSettings.cpp
        include/PluginManager/Module/ModuleBaseSettings.h src/Module/ModuleBaseSettings.cpp
        res/PluginManager.qrc
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
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
