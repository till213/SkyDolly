cmake_minimum_required(VERSION 3.5)

project(SkyDolly LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

# set(TS_FILES SkyDolly_de_DE.ts)

# SimConnect
add_library(SimConnect SHARED IMPORTED)
set_property(TARGET SimConnect PROPERTY
             IMPORTED_LOCATION  "c:/MSFS SDK/SimConnect SDK/lib/SimConnect.dll")
set_property(TARGET SimConnect PROPERTY
             IMPORTED_IMPLIB  "c:/MSFS SDK/SimConnect SDK/lib/SimConnect.lib")
include_directories(
    "c:/MSFS SDK/SimConnect SDK/include"
)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/SkyDolly.cpp
    src/SkyDolly.h
    src/SkyConnect.cpp
    src/SkyConnect.h
    src/MainWindow.ui
    src/Aircraft.cpp
    src/Aircraft.h
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SkyDolly
        ${PROJECT_SOURCES}
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    add_executable(SkyDolly
        ${PROJECT_SOURCES}
    )
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(SkyDolly PRIVATE Qt${QT_VERSION_MAJOR}::Widgets SimConnect)
