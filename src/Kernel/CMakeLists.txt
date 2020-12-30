cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

add_definitions(-DKERNEL_EXPORT)

set(PROJECT_SOURCES
    src/KernelLib.h
    src/SkyDolly.cpp
    src/SkyDolly.h
    src/Aircraft.cpp
    src/Aircraft.h
    src/Position.cpp
    src/Position.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(Kernel SHARED
        ${PROJECT_SOURCES}
    )
else()
    add_library(Kernel SHARED
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(Kernel PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(Kernel PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Kernel PROPERTIES SOVERSION 1)
