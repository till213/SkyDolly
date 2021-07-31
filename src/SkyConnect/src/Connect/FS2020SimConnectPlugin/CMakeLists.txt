set(LIBRARY_NAME "FS2020SimConnect")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model SkyConnect tsl::ordered_map SimConnect
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/FS2020SimConnectPlugin.cpp
        src/FS2020SimConnectPlugin.h
        src/FS2020SimConnectPlugin.json
        src/SimConnectType.h
        src/SimConnectPosition.cpp
        src/SimConnectPosition.h
        src/SimConnectEngineReply.cpp
        src/SimConnectEngineReply.h
        src/SimConnectEngineRequest.cpp
        src/SimConnectEngineRequest.h
        src/SimConnectPrimaryFlightControl.cpp
        src/SimConnectPrimaryFlightControl.h
        src/SimConnectSecondaryFlightControl.cpp
        src/SimConnectSecondaryFlightControl.h
        src/SimConnectAircraftHandle.cpp
        src/SimConnectAircraftHandle.h
        src/SimConnectLight.cpp
        src/SimConnectLight.h
        src/SimConnectAircraftInfo.cpp
        src/SimConnectAircraftInfo.h
        src/SimConnectFlightPlan.cpp
        src/SimConnectFlightPlan.h
        src/SimConnectSimulationTime.cpp
        src/SimConnectSimulationTime.h
        src/SimConnectAI.cpp
        src/SimConnectAI.h
        src/EventWidget.cpp
        src/EventWidget.h
)

target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})

# SimConnect
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/3rdParty/cmake")
find_package(SimConnect REQUIRED)

# Copy SimConnect.dll to bin directory (for development)
add_custom_command(TARGET ${LIBRARY_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SimConnect>
        $<TARGET_FILE_DIR:${LIBRARY_NAME}>/../../)
