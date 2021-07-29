cmake_minimum_required(VERSION 3.5)

set(SKYDOLLY_DESCRIPTION "Sky Dolly - The Black Sheep for your Flight Recordings")
project(SkyDolly LANGUAGES CXX VERSION 0.9.0 DESCRIPTION ${SKYDOLLY_DESCRIPTION})
set(PROJECT_APPLICATION_NAME "Sky Dolly")
set(PROJECT_ORGANISATION_NAME till213)

# Enables testing: 'ctest' needs to be run in the "build" directory
include(CTest)

set(CMAKE_SHARED_LIBRARY_PREFIX )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Platform identifiers

# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Write-Platform-Checks
string(COMPARE EQUAL "Windows" ${CMAKE_SYSTEM_NAME} PLATFORM_IS_WINDOWS)
string(COMPARE EQUAL "Darwin" ${CMAKE_SYSTEM_NAME} PLATFORM_IS_MACOS)
string(COMPARE EQUAL "Linux" ${CMAKE_SYSTEM_NAME} PLATFORM_IS_LINUX)
message(STATUS PLATFORM="${CMAKE_SYSTEM_NAME}")

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
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Widget)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/SkyConnect)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Plugin)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Module)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/UserInterface)

# 3rd Party
# ordered-map (Tessil)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/ordered-map)

# Documentation
add_subdirectory(${CMAKE_SOURCE_DIR}/doc)

set(TS_FILES i18n/SkyDolly_de_DE.ts)
set(OTHER_SOURCES
    README.md
    CHANGELOG.md
    BUILD.md
    CODE_OF_CONDUCT.md
    THIRD_PARTY.md
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
    qt_add_executable(SkyDolly
        ${SKYDOLLY_PLATFORM}
        ${RESOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
else()
    add_executable(SkyDolly
        ${SKYDOLLY_PLATFORM}
        ${RESOURCES}
    )
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
endif()

set(SKYDOLLY_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model Persistence SkyConnect Plugin UserInterface
)
if (${PLATFORM_IS_MACOS})
    list(APPEND SKYDOLLY_LIBS -lc++)
    set_target_properties(SkyDolly PROPERTIES
                          MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/res/Info.plist.in")
endif()

target_sources(SkyDolly
    PRIVATE
        src/main.cpp
        ${OTHER_SOURCES}
)

target_include_directories(SkyDolly PUBLIC
                          "${PROJECT_SOURCE_DIR}/src/SkyConnect/src"
                          )
target_link_libraries(SkyDolly PRIVATE ${SKYDOLLY_LIBS})
message(STATUS OUTPUT_NAME=${PROJECT_NAME})

# macOS: copy plugins into app bundle
if (${PLATFORM_IS_MACOS})

    # Connect plugins
    file(GLOB BUNDLE_IMPORT_PLUGINS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins/Connect/*")
    message(STATUS IMPORT_PLUGIN_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins/Connect/")
    target_sources(SkyDolly PUBLIC ${BUNDLE_IMPORT_PLUGINS})
    set_property(
        SOURCE ${BUNDLE_IMPORT_PLUGINS}
        PROPERTY MACOSX_PACKAGE_LOCATION "PlugIns/Connect"
    )

    # Import plugins
    file(GLOB BUNDLE_IMPORT_PLUGINS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins/Import/*")
    message(STATUS IMPORT_PLUGIN_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins/Import/")
    target_sources(SkyDolly PUBLIC ${BUNDLE_IMPORT_PLUGINS})
    set_property(
        SOURCE ${BUNDLE_IMPORT_PLUGINS}
        PROPERTY MACOSX_PACKAGE_LOCATION "PlugIns/Import"
    )

    # Export
    file(GLOB BUNDLE_EXPORT_PLUGINS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins/Export/*")
    message(STATUS EXPORT_PLUGIN_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Plugins/Export/")
    target_sources(SkyDolly PUBLIC ${BUNDLE_EXPORT_PLUGINS})
    set_property(
        SOURCE ${BUNDLE_EXPORT_PLUGINS}
        PROPERTY MACOSX_PACKAGE_LOCATION "PlugIns/Export"
    )
endif()



