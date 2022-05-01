set(LIBRARY_NAME "Model")

add_definitions(-DMODEL_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Sky::Kernel
)

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/ModelLib.h
        src/Logbook.h src/Logbook.cpp
        src/Flight.h src/Flight.cpp
        src/FlightSummary.h src/FlightSummary.cpp
        src/FlightDate.h src/FlightDate.cpp
        src/FlightCondition.h src/FlightCondition.cpp
        src/Aircraft.h src/Aircraft.cpp
        src/TimeVariableData.h src/TimeVariableData.cpp
        src/Position.h src/Position.cpp
        src/PositionData.h src/PositionData.cpp
        src/InitialPosition.h src/InitialPosition.cpp
        src/Engine.h src/Engine.cpp
        src/EngineData.h src/EngineData.cpp
        src/PrimaryFlightControl.h src/PrimaryFlightControl.cpp
        src/PrimaryFlightControlData.h src/PrimaryFlightControlData.cpp
        src/SecondaryFlightControl.h src/SecondaryFlightControl.cpp
        src/SecondaryFlightControlData.h src/SecondaryFlightControlData.cpp
        src/AircraftHandle.h src/AircraftHandle.cpp
        src/AircraftHandleData.h src/AircraftHandleData.cpp
        src/Light.h src/Light.cpp
        src/LightData.h src/LightData.cpp
        src/AircraftType.h src/AircraftType.cpp
        src/AircraftInfo.h src/AircraftInfo.cpp
        src/Waypoint.h src/Waypoint.cpp
        src/FlightPlan.h src/FlightPlan.cpp
        src/SimType.h src/SimVar.h
        src/SkySearch.h src/SkySearch.cpp
)

target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${LIBRARY_DEPENDENCIES}
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
