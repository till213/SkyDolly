set(MODULE_NAME "MSFSSimConnect")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})
# Don't define MIN, MAX macros (in windows.h)
add_definitions(-DNOMINMAX)

target_sources(${MODULE_NAME}
    PRIVATE
        src/SimVar/Position/SimConnectPositionCommon.h
        src/SimVar/Position/SimConnectPositionInfo.h
        src/SimVar/Position/SimConnectPositionUser.h
        src/SimVar/Position/SimConnectPositionAi.h
        src/SimVar/Position/SimConnectPositionAll.h
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
        src/SimVar/SimConnectSecondaryFlightControl.h src/SimVar/SimConnectSecondaryFlightControl.cpp
        src/SimVar/SimConnectAircraftHandle.h src/SimVar/SimConnectAircraftHandle.cpp
        src/SimVar/SimConnectLight.h src/SimVar/SimConnectLight.cpp
        src/SimVar/SimConnectAircraftInfo.h src/SimVar/SimConnectAircraftInfo.cpp
        src/SimVar/SimConnectFlightPlan.h src/SimVar/SimConnectFlightPlan.cpp
        src/SimVar/SimConnectSimulationTime.h src/SimVar/SimConnectSimulationTime.cpp
        src/SimVar/SimConnectLocation.h src/SimVar/SimConnectLocation.cpp
        src/SimVar/SimConnectVariables.h src/SimVar/SimConnectVariables.cpp
        src/SimVar/SimulationVariables.h
        src/Event/EventWidget.h src/Event/EventWidget.cpp
        src/Event/SimConnectEvent.h
        src/SimConnectType.h
        src/SimConnectAi.h src/SimConnectAi.cpp        
        src/MSFSSimConnectPlugin.h src/MSFSSimConnectPlugin.cpp
        src/MSFSSimConnectPlugin.json
        doc/SimVars.md
)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
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
add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:MSFS::SimConnect>
        $<TARGET_FILE_DIR:${MODULE_NAME}>/../../)
