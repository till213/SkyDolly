cmake_minimum_required(VERSION 3.16...3.23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CMakePrintHelpers)
include(CheckIPOSupported)

set(APP_NAME "SkyDolly")

project(
    ${APP_NAME}
    VERSION 0.12.0
    DESCRIPTION "Sky Dolly - The Black Sheep for your Flight Recordings"
    HOMEPAGE_URL "https://github.com/till213/SkyDolly"
    LANGUAGES CXX    
)
# Used to populate the Info.plist.in and SkyDolly.rc.in
set(PROJECT_APPLICATION_NAME "Sky Dolly")
set(PROJECT_ORGANISATION_NAME till213)

include(InitSubmodules)

# Enables testing: 'ctest' needs to be run in the "build" directory
include(CTest)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX "")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform identifiers

# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Write-Platform-Checks
string(COMPARE EQUAL "Windows" ${CMAKE_SYSTEM_NAME} PLATFORM_IS_WINDOWS)
string(COMPARE EQUAL "Darwin" ${CMAKE_SYSTEM_NAME} PLATFORM_IS_MACOS)
string(COMPARE EQUAL "Linux" ${CMAKE_SYSTEM_NAME} PLATFORM_IS_LINUX)
cmake_print_variables(CMAKE_SYSTEM_NAME)

# Binary output locations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
if(${PLATFORM_IS_MACOS})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${APP_NAME}.app/Contents/Frameworks)
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# Definitions

# Debug output
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()
# No use of deprecated Qt functionality before Qt 5.15
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x051500)

# Qt
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Sql REQUIRED)

# 3rd Party
# ordered-map (Tessil)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/ordered-map)

# GeographicLib (https://github.com/geographiclib/geographiclib)
set(GEOGRAPHICLIB_LIB_TYPE SHARED)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/geographiclib EXCLUDE_FROM_ALL)
set_target_properties(GeographicLib
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enabled link time optimisation (LTO) for release build, if available
if(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
    check_ipo_supported(RESULT ipo_support OUTPUT output LANGUAGES CXX)
    if(ipo_support)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "IPO SUPPORT enabled")
    else()
        message(WARNING "IPO is not supported: ${output}")
    endif()
else()
    message(STATUS "IPO SUPPORT disabled for debug builds")
endif()

# https://jonathanhamberg.com/post/cmake-embedding-git-hash/
set(PRE_CONFIGURE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include(cmake/CheckGit.cmake)
CheckGitSetup()

# Only enable auto-moccing for our own libraries and executables
# (not for the third-party libraries above)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${APP_NAME})
    qt_create_translation(QM_FILES
        ${CMAKE_SOURCE_DIR}/src
        i18n/${APP_NAME}_de_DE.ts
    )
else()
    add_executable(${APP_NAME})
    qt5_create_translation(QM_FILES
        ${CMAKE_SOURCE_DIR}/src
        i18n/${APP_NAME}_de_DE.ts
    )
endif()

set_target_properties(${APP_NAME}
    PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/res/Info.plist.in"
)

# Resource (data) files
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/geoids)
if (${PLATFORM_IS_MACOS})
    target_link_libraries(${APP_NAME}
        PRIVATE
            Geoids
    )
    include(CreateAppIconOnMacOS)
    create_app_icon_on_macos(INPUT "res/SkyDolly.png" OUTPUT APP_ICON_PATH)
    set_source_files_properties(${APP_ICON_PATH}
        PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(${APP_NAME}
        PRIVATE
            ${APP_ICON_PATH}
    )
elseif(${PLATFORM_IS_WINDOWS})
    # KNOWN ISSUE: windres.exe (MinGW on Windows) does not properly handle include paths with spaces
    #              Also refer e.g. to https://bugreports.qt.io/browse/QTBUG-62918
    # WORKAROUND: Place the Sky Dolly sources into a path without spaces
    # Resource file (*.rc)
    configure_file("${PROJECT_SOURCE_DIR}/res/SkyDolly.rc.in"
                   "${PROJECT_SOURCE_DIR}/res/SkyDolly.rc" @ONLY)
    add_library(resources OBJECT "${PROJECT_SOURCE_DIR}/res/SkyDolly.rc")
    target_include_directories(resources PRIVATE "${PROJECT_SOURCE_DIR}/res/")
    target_sources(${APP_NAME}
        PRIVATE
            $<TARGET_OBJECTS:resources>
    )
endif()

target_sources(${APP_NAME}
    PRIVATE
        src/main.cpp
        BUILD.md
        CHANGELOG.md
        CODE_OF_CONDUCT.md
        README.md
        RELEASE.md
        SHASUM256.md
        THIRD_PARTY.md
        doc/SkyDolly.simvars
        res/Info.plist.in
        res/SkyDolly.rc.in
)

target_include_directories(${APP_NAME}
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated
)
target_link_libraries(${APP_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Sky::Kernel
        Sky::Model
        Sky::Persistence
        Sky::PluginManager
        Sky::UserInterface
)

# Plugins
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Plugins)

# Documentation
add_subdirectory(${CMAKE_SOURCE_DIR}/doc)
