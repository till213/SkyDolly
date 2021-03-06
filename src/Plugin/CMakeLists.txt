add_definitions(-DPLUGIN_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(Plugin SHARED "")
else()
    add_library(Plugin SHARED "")
endif()

target_sources(Plugin
    PRIVATE
        src/PluginLib.h
        src/PluginManager.cpp
        src/PluginManager.h
        src/PluginIntf.h
        src/ExportIntf.h
        src/ImportIntf.h
        src/PluginBase.cpp
        src/PluginBase.h
)

set(PLUGIN_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model
)
if (${PLATFORM_IS_MACOS})
    list(APPEND PLUGIN_LIBS -lc++)
endif()

target_link_libraries(Plugin PRIVATE ${PLUGIN_LIBS})
set_target_properties(Plugin PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Plugin PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(Plugin PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# Plugins
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Export)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Import)
