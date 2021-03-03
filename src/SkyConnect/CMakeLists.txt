# SimConnect
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/3rdParty/cmake")
find_package(SimConnect REQUIRED)

add_definitions(-DSKYCONNECT_EXPORT)

set(PROJECT_SOURCES
    src/SkyConnectLib.h
    src/SimConnectAircraftData.cpp
    src/SimConnectAircraftData.h
    src/SimConnectAircraftInfo.cpp
    src/SimConnectAircraftInfo.h
    src/SkyConnect.cpp
    src/SkyConnect.h
    src/SkyConnectImpl.cpp
    src/SkyConnectImpl.h
    src/SimConnectDataDefinition.h
    src/Connect.h
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

# Copy SimConnect.dll to bin directory (for development)
add_custom_command(TARGET SkyConnect POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SimConnect>
        $<TARGET_FILE_DIR:SkyConnect>)

