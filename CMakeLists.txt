cmake_minimum_required(VERSION 3.16...3.23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(CMakePrintHelpers)
include(CheckIPOSupported)

set(APP_NAME "SkyDolly")
set(APP_PROJECT_NAME "SkyDollyProject")
# Used to populate the Info.plist.in and SkyDolly.rc.in
set(PROJECT_APPLICATION_NAME "Sky Dolly")
set(PROJECT_ORGANISATION_NAME till213)
set(PROJECT_BUNDLE_IDENTIFIER net.${PROJECT_ORGANISATION_NAME}.${APP_NAME})

project(
    ${APP_NAME}
    VERSION 0.19.2
    DESCRIPTION "Sky Dolly - The Black Sheep for Your Flight Recordings"
    HOMEPAGE_URL "https://github.com/till213/SkyDolly"
    LANGUAGES CXX
)

include(InitSubmodules)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX "")
set(CMAKE_CXX_STANDARD 20)
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
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.github/till213/skydolly")
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# Definitions

# Debug output
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()
# No use of deprecated Qt functionality before Qt 6.7
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060700)
 
# Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql LinguistTools)

# 3rd Party
# ordered-map (Tessil)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rdParty/ordered-map EXCLUDE_FROM_ALL)

# GeographicLib (https://github.com/geographiclib/geographiclib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rdParty/geographiclib EXCLUDE_FROM_ALL)
set_target_properties(GeographicLib_SHARED
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Cpptrace
if(${PLATFORM_IS_WINDOWS} AND MINGW)
    # While MinGW provides an addr2line executable we do not want to ship / use it
    set(CPPTRACE_UNWIND_WITH_UNWIND ON CACHE BOOL "Use the Unwind backend for stack unwinding [Windows, MinGW]")
    set(CPPTRACE_GET_SYMBOLS_WITH_DBGHELP ON CACHE BOOL "Use DbgHelp for getting the symbols [Windows, MinGW]")
    message("CPPTRACE: Set Windows specific MINGW settings")
elseif(${PLATFORM_IS_WINDOWS} AND MSVC)
    set(CPPTRACE_UNWIND_WITH_WINAPI ON CACHE BOOL "Use the WinAPI backend for stack unwinding [Windows, MSVC]")
    set(CPPTRACE_GET_SYMBOLS_WITH_DBGHELP ON CACHE BOOL "Use DbgHelp for getting the symbols [Windows, MSVC]")
    message("CPPTRACE: Set Windows specific MSVC settings")
else()
    message("CPPTRACE: Let cpptrace automatically set configuration")
endif()
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdParty/cpptrace EXCLUDE_FROM_ALL)

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

# Only enable auto-moccing for our own libraries and executables
# (not for the third-party libraries above)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Enables testing: 'ctest' needs to be run in the "build" directory
include(CTest)

# Application libraries
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Kernel)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Model)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Persistence)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Flight)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Widget)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/PluginManager)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/UserInterface)

# Application
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/SkyDolly)

# Resource (data) files
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rdParty/geoids)
include(src/Persistence/cmake/Locations.cmake)

add_custom_target(${APP_PROJECT_NAME})
target_sources(${APP_PROJECT_NAME}
    PRIVATE
        ABOUT.md
        BUILD.md
        CHANGELOG.md
        CODE_OF_CONDUCT.md
        README.md
        RELEASE.md
        SECURITY.md
        SHASUM256.md
        THIRD_PARTY.md
        doc/Design/Architecture.md
        doc/Design/README.md
        doc/Guidelines/User-Interface-Guidelines.md
        doc/SkyDolly.simvars
        doc/LICENSE
        doc/SQL/Timelapse-Camera-Pan.sql
        doc/SQL/Timelapse-One-Year-in-New-York.sql
        doc/SQL/Flight-Analysis.sql
)

# Plugins
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/Plugins)

# Documentation
option(SKY_DOXY_DOC "Build Doxygen documentation" OFF)
if(SKY_DOXY_DOC)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/doc)
endif()

# Tests
option(SKY_TESTS "Build Sky Dolly unit tests" OFF)
if(SKY_TESTS)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
endif()
