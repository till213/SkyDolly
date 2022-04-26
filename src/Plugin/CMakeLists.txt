set(LIBRARY_NAME "Plugin")

add_definitions(-DPLUGIN_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(Plugin SHARED "")
else()
    add_library(Plugin SHARED "")
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/PluginLib.h
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

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Flight Persistence SkyConnect Widget
)
if (${PLATFORM_IS_MACOS})
    list(APPEND PLUGIN_LIBS -lc++)
endif()

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

# Plugins
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Export)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Import)
