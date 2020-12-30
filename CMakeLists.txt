cmake_minimum_required(VERSION 3.5)

project(SkyDolly LANGUAGES CXX VERSION 0.1 DESCRIPTION "SkyDolly records flight simulator flights")

set(CMAKE_SHARED_LIBRARY_PREFIX )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Qt
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

# Application libraries
add_subdirectory(${CMAKE_SOURCE_DIR}/src/Kernel)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/SkyConnect)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/UserInterface)

set(TS_FILES i18n/SkyDolly_de_DE.ts)

set(PROJECT_SOURCES
    src/main.cpp
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SkyDolly
        WIN32
        ${PROJECT_SOURCES}
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
else()
    add_executable(SkyDolly
        WIN32
        ${PROJECT_SOURCES}
    )
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${TS_FILES})
endif()

target_include_directories(SkyDolly PUBLIC
                          "${PROJECT_SOURCE_DIR}/src/SkyConnect/src"
                          )

target_link_libraries(SkyDolly PRIVATE Qt${QT_VERSION_MAJOR}::Widgets UserInterface)
