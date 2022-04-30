set(LIBRARY_NAME "MSFSSimConnect")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} MODULE "")
else()
    add_library(${LIBRARY_NAME} MODULE "")
endif()
add_library(SD::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Kernel Model SkyConnect tsl::ordered_map SimConnect
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/MSFSSimConnectPlugin.h src/MSFSSimConnectPlugin.cpp
        src/SimConnectType.h
        src/SimConnectPositionRequest.h src/SimConnectPositionRequest.cpp
        src/SimConnectPositionReply.h src/SimConnectPositionReply.cpp
        src/SimConnectEngineReply.h src/SimConnectEngineReply.cpp
        src/SimConnectEngineRequest.h src/SimConnectEngineRequest.cpp
        src/SimConnectPrimaryFlightControl.h src/SimConnectPrimaryFlightControl.cpp
        src/SimConnectSecondaryFlightControl.h src/SimConnectSecondaryFlightControl.cpp
        src/SimConnectAircraftHandle.h src/SimConnectAircraftHandle.cpp
        src/SimConnectLight.h src/SimConnectLight.cpp
        src/SimConnectAircraftInfo.h src/SimConnectAircraftInfo.cpp
        src/SimConnectFlightPlan.h src/SimConnectFlightPlan.cpp
        src/SimConnectSimulationTime.h src/SimConnectSimulationTime.cpp
        src/SimConnectAI.h src/SimConnectAI.cpp
        src/EventWidget.h src/EventWidget.cpp
        src/MSFSSimConnectPlugin.json
)

target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)

# SimConnect
find_package(SimConnect REQUIRED)

# Copy SimConnect.dll to bin directory (for development)
add_custom_command(TARGET ${LIBRARY_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SimConnect>
        $<TARGET_FILE_DIR:${LIBRARY_NAME}>/../../)
