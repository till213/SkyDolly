add_definitions(-DSKYCONNECT_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(SkyConnect SHARED "")
else()
    add_library(SkyConnect SHARED "")
endif()

target_sources(SkyConnect
    PRIVATE
        src/SkyConnectLib.h
        src/SkyConnectManager.cpp
        src/SkyConnectManager.h
        src/SkyConnectIntf.h
        src/Connect.h
        src/AbstractSkyConnect.cpp
        src/AbstractSkyConnect.h
)

set(SKYCONNECT_LIBS
    Qt${QT_VERSION_MAJOR}::Core Kernel Model
)
if (${PLATFORM_IS_MACOS})
    list(APPEND SKYCONNECT_LIBS -lc++)
endif()

target_link_libraries(SkyConnect PRIVATE ${SKYCONNECT_LIBS})
set_target_properties(SkyConnect PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(SkyConnect PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

# Plugins
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Connect)
