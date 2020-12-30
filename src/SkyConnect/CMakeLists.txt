cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

# SimConnect
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/3rdParty/cmake")
find_package(SimConnect REQUIRED)

add_definitions(-DSKYCONNECT_EXPORT)

set(PROJECT_SOURCES
    src/SkyConnectLib.h
    src/SimConnectPosition.cpp
    src/SimConnectPosition.h
    src/SimConnectAircraftInfo.cpp
    src/SimConnectAircraftInfo.h
    src/SkyConnect.cpp
    src/SkyConnect.h
    src/SkyConnectImpl.cpp
    src/SkyConnectImpl.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(SkyDolly SHARED
        ${PROJECT_SOURCES}
    )
else()
    add_library(SkyConnect SHARED
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(SkyConnect PRIVATE Qt${QT_VERSION_MAJOR}::Core SimConnect Kernel)
set_target_properties(SkyConnect PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(SkyConnect PROPERTIES SOVERSION 1)

