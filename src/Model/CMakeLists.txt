set(LIBRARY_NAME "Model")

add_definitions(-DMODEL_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Sql Kernel
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/ModelLib.h
        src/Logbook.cpp
        src/Logbook.h
        src/Flight.cpp
        src/Flight.h
        src/FlightSummary.cpp
        src/FlightSummary.h
        src/FlightDate.cpp
        src/FlightDate.h
        src/FlightCondition.cpp
        src/FlightCondition.h
        src/Aircraft.cpp
        src/Aircraft.h
        src/TimeVariableData.cpp
        src/TimeVariableData.h
        src/Position.cpp
        src/Position.h
        src/PositionData.cpp
        src/PositionData.h
        src/InitialPosition.cpp
        src/InitialPosition.h
        src/Engine.cpp
        src/Engine.h
        src/EngineData.cpp
        src/EngineData.h
        src/PrimaryFlightControl.cpp
        src/PrimaryFlightControl.h
        src/PrimaryFlightControlData.cpp
        src/PrimaryFlightControlData.h
        src/SecondaryFlightControl.cpp
        src/SecondaryFlightControl.h
        src/SecondaryFlightControlData.cpp
        src/SecondaryFlightControlData.h
        src/AircraftHandle.cpp
        src/AircraftHandle.h
        src/AircraftHandleData.cpp
        src/AircraftHandleData.h
        src/Light.cpp
        src/Light.h
        src/LightData.cpp
        src/LightData.h
        src/AircraftType.cpp
        src/AircraftType.h
        src/AircraftInfo.cpp
        src/AircraftInfo.h
        src/Waypoint.cpp
        src/Waypoint.h
        src/FlightPlan.cpp
        src/FlightPlan.h
        src/SimVar.h
        src/SimType.h
        src/SkySearch.cpp
        src/SkySearch.h
)

target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
set_target_properties(${LIBRARY_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${LIBRARY_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

