set(MODULE_NAME "MSFSSimConnect")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${MODULE_NAME} MODULE)
else()
    add_library(${MODULE_NAME} MODULE)
endif()
add_library(Sky::${MODULE_NAME} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
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
# SimConnect
find_package(SimConnect REQUIRED)
target_link_libraries(${MODULE_NAME}
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        SimConnect
        tsl::ordered_map
        Sky::Kernel
        Sky::Model
        Sky::PluginManager
)

# Copy SimConnect.dll to bin directory
add_custom_command(TARGET ${LIBRARY_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SimConnect>
        $<TARGET_FILE_DIR:${LIBRARY_NAME}>/../../
)
