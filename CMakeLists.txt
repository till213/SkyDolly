cmake_minimum_required(VERSION 3.5)

set(SKYDOLLY_DESCRIPTION "Sky Dolly - The black sheep for your flight recordings")
project(SkyDolly LANGUAGES CXX VERSION 0.5.0 DESCRIPTION ${SKYDOLLY_DESCRIPTION})
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

# Definitions

# Debug output
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()
# No use of deprecated Qt functionality
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x000000)

# Qt
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Sql REQUIRED)

# Application libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Kernel)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/KernelTest)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Model)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/ModelTest)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/SkyConnect)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/UserInterface)

# Documentation
add_subdirectory(${CMAKE_SOURCE_DIR}/doc)

set(TS_FILES i18n/SkyDolly_de_DE.ts)
set(OTHER_SOURCES
    README.md
    CHANGELOG.md
    BUILD.md
    CODE_OF_CONDUCT.md
    doc/SkyDolly.simvars
    ${CMAKE_SOURCE_DIR}/res/Info.plist.in
)

# Resource file (*.rc)
if (WIN32)
    configure_file(${PROJECT_SOURCE_DIR}/res/SkyDolly.rc.in
                   ${PROJECT_SOURCE_DIR}/res/SkyDolly.rc @ONLY)
    add_library(resources OBJECT ${PROJECT_SOURCE_DIR}/res/SkyDolly.rc)
    target_include_directories(resources PRIVATE ${PROJECT_SOURCE_DIR}/res/)
    set(RESOURCES $<TARGET_OBJECTS:resources>)
endif()

if(WIN32)
    set(SKYDOLLY_PLATFORM WIN32)
elseif(APPLE)
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
    Qt${QT_VERSION_MAJOR}::Widgets UserInterface Kernel Model SkyConnect
)
if (APPLE)
    list(APPEND SKYDOLLY_LIBS -lc++)
    set_target_properties(SkyDolly PROPERTIES
                          MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/res/Info.plist.in")
endif()

target_sources(SkyDolly
    PRIVATE
        src/main.cpp
        src/SkyDollyApplication.cpp
        src/SkyDollyApplication.h
        ${TS_FILES}
        ${OTHER_SOURCES}
)

target_include_directories(SkyDolly PUBLIC
                          "${PROJECT_SOURCE_DIR}/src/SkyConnect/src"
                          )
target_link_libraries(SkyDolly PRIVATE ${SKYDOLLY_LIBS})
message(STATUS OUTPUT_NAME=${PROJECT_NAME})
