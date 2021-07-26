set(LIBRARY_NAME "SkyConnect")

add_definitions(-DSKYCONNECT_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Core Kernel Model
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/SkyConnectLib.h
        src/SkyConnectManager.cpp
        src/SkyConnectManager.h
        src/SkyConnectIntf.h
        src/Connect.h
        src/AbstractSkyConnect.cpp
        src/AbstractSkyConnect.h
)

target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
set_target_properties(${LIBRARY_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${LIBRARY_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

# Plugins
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Connect)
