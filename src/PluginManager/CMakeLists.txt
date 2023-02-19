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
        include/PluginManager/PluginIntf.h

        include/PluginManager/SkyConnectManager.h src/SkyConnectManager.cpp
        include/PluginManager/AbstractSkyConnect.h src/AbstractSkyConnect.cpp
        include/PluginManager/PluginManager.h src/PluginManager.cpp
        include/PluginManager/Export.h src/Export.cpp
        include/PluginManager/PluginBase.h src/PluginBase.cpp
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
        include/PluginManager/LocationExportIntf.h
        include/PluginManager/LocationExportPluginBase.h src/LocationExportPluginBase.cpp
        include/PluginManager/LocationExportPluginBaseSettings.h src/LocationExportPluginBaseSettings.cpp
        include/PluginManager/BasicLocationExportDialog.h src/BasicLocationExportDialog.cpp src/BasicLocationExportDialog.ui
        include/PluginManager/LocationImportIntf.h
        include/PluginManager/LocationImportPluginBase.h src/LocationImportPluginBase.cpp
        include/PluginManager/LocationImportPluginBaseSettings.h src/LocationImportPluginBaseSettings.cpp
        include/PluginManager/BasicLocationImportDialog.h src/BasicLocationImportDialog.cpp src/BasicLocationImportDialog.ui
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
