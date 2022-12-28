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
        src/MSFSSimConnectPlugin.h src/MSFSSimConnectPlugin.cpp
        src/SimConnectType.h
        src/SimConnectPositionRequest.h src/SimConnectPositionRequest.cpp
        src/SimConnectPositionReply.h src/SimConnectPositionReply.cpp
        src/SimConnectEngineReply.h src/SimConnectEngineReply.cpp
        src/SimConnectEngineRequest.h src/SimConnectEngineRequest.cpp
        src/SimConnectPrimaryFlightControlAnimation.h
        src/SimConnectPrimaryFlightControl.h
        src/SimConnectPrimaryFlightControlReply.h
        src/SimConnectSecondaryFlightControl.h src/SimConnectSecondaryFlightControl.cpp
        src/SimConnectAircraftHandle.h src/SimConnectAircraftHandle.cpp
        src/SimConnectLight.h src/SimConnectLight.cpp
        src/SimConnectAircraftInfo.h src/SimConnectAircraftInfo.cpp
        src/SimConnectFlightPlan.h src/SimConnectFlightPlan.cpp
        src/SimConnectSimulationTime.h src/SimConnectSimulationTime.cpp
        src/SimConnectLocation.h src/SimConnectLocation.cpp
        src/SimConnectVariables.h src/SimConnectVariables.cpp
        src/SimConnectAi.h src/SimConnectAi.cpp
        src/EventWidget.h src/EventWidget.cpp
        src/SimConnectEvent.h
        src/MSFSSimConnectPlugin.json
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

# Copy SimConnect.dll to bin directory
add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:MSFS::SimConnect>
        $<TARGET_FILE_DIR:${MODULE_NAME}>/../../)
