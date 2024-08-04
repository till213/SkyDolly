set(MODULE_NAME "MSFSSimConnect")

qt_add_library(${MODULE_NAME} MODULE)
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})
# Don't define MIN, MAX macros (in windows.h)
add_definitions(-DNOMINMAX)

target_sources(${MODULE_NAME}
    PRIVATE
        src/SimVar/Position/SimConnectPositionCommon.h
        src/SimVar/Position/SimConnectPositionInfo.h
        src/SimVar/Position/SimConnectPositionAll.h
        src/SimVar/Attitude/SimConnectAttitudeCommon.h
        src/SimVar/Attitude/SimConnectAttitudeInfo.h
        src/SimVar/Attitude/SimConnectAttitudeAll.h
        src/SimVar/PositionAndAttitude/SimConnectPositionAndAttitudeUser.h
        src/SimVar/PositionAndAttitude/SimConnectPositionAndAttitudeAi.h
        src/SimVar/PositionAndAttitude/SimConnectPositionAndAttitudeAll.h
        src/SimVar/Engine/SimConnectEngineCommon.h
        src/SimVar/Engine/SimConnectEngineCore.h
        src/SimVar/Engine/SimConnectEngineEvent.h
        src/SimVar/Engine/SimConnectEngineAll.h
        src/SimVar/Engine/SimConnectEngineUser.h
        src/SimVar/Engine/SimConnectEngineAi.h
        src/SimVar/PrimaryFlightControl/SimConnectPrimaryFlightControlAnimation.h
        src/SimVar/PrimaryFlightControl/SimConnectPrimaryFlightControlEvent.h
        src/SimVar/PrimaryFlightControl/SimConnectPrimaryFlightControlAll.h
        src/SimVar/PrimaryFlightControl/SimConnectPrimaryFlightControlAi.h
        src/SimVar/SecondaryFlightControl/SimConnectSecondaryFlightControlAnimation.h
        src/SimVar/SecondaryFlightControl/SimConnectSecondaryFlightControlEvent.h
        src/SimVar/SecondaryFlightControl/SimConnectSecondaryFlightControlAll.h
        src/SimVar/SecondaryFlightControl/SimConnectSecondaryFlightControlAi.h
        src/SimVar/AircraftHandle/SimConnectAircraftHandleAnimation.h
        src/SimVar/AircraftHandle/SimConnectAircraftHandleCommon.h
        src/SimVar/AircraftHandle/SimConnectAircraftHandleCore.h
        src/SimVar/AircraftHandle/SimConnectAircraftHandleCoreEvent.h
        src/SimVar/AircraftHandle/SimConnectAircraftHandleEvent.h
        src/SimVar/AircraftHandle/SimConnectAircraftHandleInfo.h
        src/SimVar/AircraftHandle/SimConnectAircraftHandleAll.h
        src/SimVar/AircraftHandle/SimConnectAircraftHandleUser.h
        src/SimVar/AircraftHandle/SimConnectAircraftHandleAi.h
        src/SimVar/Light/SimConnectLightEvent.h
        src/SimVar/Light/SimConnectLightAll.h
        src/SimVar/Light/SimConnectLightAi.h
        src/SimVar/Sensor/SimConnectAltitudeSensor.h
        src/SimVar/Sensor/SimConnectReplaySensor.h
        src/SimVar/SimConnectAircraftInfo.h src/SimVar/SimConnectAircraftInfo.cpp
        src/SimVar/SimConnectFlightInfo.h src/SimVar/SimConnectFlightInfo.cpp
        src/SimVar/SimConnectFlightPlan.h src/SimVar/SimConnectFlightPlan.cpp
        src/SimVar/SimConnectSimulationTime.h src/SimVar/SimConnectSimulationTime.cpp
        src/SimVar/SimConnectTimeZoneInfo.h src/SimVar/SimConnectTimeZoneInfo.cpp
        src/SimVar/SimConnectLocation.h src/SimVar/SimConnectLocation.cpp
        src/SimVar/SimConnectVariables.h src/SimVar/SimConnectVariables.cpp
        src/SimVar/SimulationVariables.h
        src/SimVar/SimConnectType.h
        src/Event/SimConnectEvent.h
        src/Event/EventState.h
        src/Event/EventWidget.h src/Event/EventWidget.cpp
        src/Event/EventStateHandler.h
        src/Event/InputEvent.h src/Event/InputEvent.cpp
        src/Event/SimulationRate.h
        src/Event/SimulationTime.h
        src/SimConnectAi.h src/SimConnectAi.cpp        
        src/MSFSSimConnectPlugin.h src/MSFSSimConnectPlugin.cpp
        src/MSFSSimConnectSettings.h src/MSFSSimConnectSettings.cpp
        src/MSFSSimConnectOptionWidget.h src/MSFSSimConnectOptionWidget.cpp src/MSFSSimConnectOptionWidget.ui
        src/MSFSSimConnectPlugin.json
        res/SimConnect.cfg
        doc/SimVars.md
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt6::Widgets
        MSFS::SimConnect
        tsl::ordered_map
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)
target_include_directories(${MODULE_NAME}
    PRIVATE
        src
        src/SimVar
)

# Copy SimConnect.dll to bin directory
add_custom_command(
    TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:MSFS::SimConnect>
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Copy example SimConnect.cfg
add_custom_command(
    TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/res/SimConnect.cfg
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
