cmake_minimum_required(VERSION 3.16...3.23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CMakePrintHelpers)

set(APP_NAME "SkyDolly")

project(
    ${APP_NAME}
    VERSION 0.11.0
    LANGUAGES CXX
    DESCRIPTION "Sky Dolly - The Black Sheep for your Flight Recordings"
)
# Used to populate the Info.plist.in and SkyDolly.rc.in
set(PROJECT_APPLICATION_NAME "Sky Dolly")
set(PROJECT_ORGANISATION_NAME till213)

include(InitSubmodules)

# Enables testing: 'ctest' needs to be run in the "build" directory
include(CTest)

set(CMAKE_SHARED_LIBRARY_PREFIX )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Platform identifiers

# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Write-Platform-Checks
string(COMPARE EQUAL "Windows" ${CMAKE_SYSTEM_NAME} PLATFORM_IS_WINDOWS)
string(COMPARE EQUAL "Darwin" ${CMAKE_SYSTEM_NAME} PLATFORM_IS_MACOS)
string(COMPARE EQUAL "Linux" ${CMAKE_SYSTEM_NAME} PLATFORM_IS_LINUX)
cmake_print_variables(CMAKE_SYSTEM_NAME)

# Binary output locations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
if (${PLATFORM_IS_MACOS})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${APP_NAME}.app/Contents/Frameworks)
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# Definitions

# Debug output
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()
# No use of deprecated Qt functionality before Qt 5.15
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x051500)

# Qt
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Sql REQUIRED)

# Application libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Kernel)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/KernelTest)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Model)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/ModelTest)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Persistence)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Flight)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Widget)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/PluginManager)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Module)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/UserInterface)

# 3rd Party
# ordered-map (Tessil)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/ordered-map)

# GeographicLib (https://github.com/geographiclib/geographiclib)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/geographiclib EXCLUDE_FROM_ALL)
set_target_properties(GeographicLib_SHARED
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Documentation
add_subdirectory(${CMAKE_SOURCE_DIR}/doc)

set(TS_FILES i18n/${APP_NAME}_de_DE.ts)
set(OTHER_SOURCES
    README.md
    CHANGELOG.md
    BUILD.md
    CODE_OF_CONDUCT.md
    THIRD_PARTY.md
    SHASUM256.md
    doc/SkyDolly.simvars
    res/Info.plist.in
    res/SkyDolly.rc.in
)

# Resource file (*.rc)
if (${PLATFORM_IS_WINDOWS})
    configure_file(${PROJECT_SOURCE_DIR}/res/SkyDolly.rc.in
                   ${PROJECT_SOURCE_DIR}/res/SkyDolly.rc @ONLY)
    add_library(resources OBJECT ${PROJECT_SOURCE_DIR}/res/SkyDolly.rc)
    target_include_directories(resources PRIVATE ${PROJECT_SOURCE_DIR}/res/)
    set(RESOURCES $<TARGET_OBJECTS:resources>)
endif()

if (${PLATFORM_IS_WINDOWS})
    set(SKYDOLLY_PLATFORM WIN32)
elseif (${PLATFORM_IS_MACOS})
    set(SKYDOLLY_PLATFORM MACOSX_BUNDLE)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${APP_NAME}
        ${SKYDOLLY_PLATFORM}
        ${RESOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
else()
    add_executable(${APP_NAME}
        ${SKYDOLLY_PLATFORM}
        ${RESOURCES}
    )
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
endif()

# Data files
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/geoids)

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets
    Sky::Kernel
    Sky::Model
    Sky::Persistence
    Sky::PluginManager
    Sky::UserInterface
)
if (${PLATFORM_IS_MACOS})
    set_target_properties(
        ${APP_NAME}
        PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/res/Info.plist.in"
    )
    target_link_libraries(
        ${APP_NAME}
        PRIVATE
            Geoids
    )
endif()

target_sources(${APP_NAME}
    PRIVATE
        src/main.cpp
        ${OTHER_SOURCES}
)

target_include_directories(${APP_NAME}
    PRIVATE
        "${PROJECT_SOURCE_DIR}/src/PluginManager/src"
)
target_link_libraries(${APP_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)

# Plugins
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Plugins)
