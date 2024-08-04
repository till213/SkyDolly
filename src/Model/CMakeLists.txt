set(LIBRARY_NAME "Model")

qt_add_library(${LIBRARY_NAME} SHARED
    src/TimeZoneInfo.cpp)
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        MODEL_EXPORT
)
target_sources(${LIBRARY_NAME}
    PRIVATE
        include/Model/ModelLib.h
        include/Model/Data.h
        include/Model/Logbook.h src/Logbook.cpp
        include/Model/AbstractComponent.h
        include/Model/Flight.h src/Flight.cpp
        include/Model/FlightData.h 
        include/Model/FlightSummary.h src/FlightSummary.cpp
        include/Model/FlightDate.h src/FlightDate.cpp
        include/Model/FlightCondition.h src/FlightCondition.cpp
        include/Model/Aircraft.h src/Aircraft.cpp
        include/Model/TimeVariableData.h
        include/Model/Position.h src/Position.cpp
        include/Model/PositionData.h src/PositionData.cpp
        include/Model/AltitudeSensorData.h
        include/Model/Attitude.h src/Attitude.cpp
        include/Model/AttitudeData.h src/AttitudeData.cpp
        include/Model/InitialPosition.h src/InitialPosition.cpp
        include/Model/Engine.h src/Engine.cpp
        include/Model/EngineData.h src/EngineData.cpp
        include/Model/PrimaryFlightControl.h src/PrimaryFlightControl.cpp
        include/Model/PrimaryFlightControlData.h
        include/Model/SecondaryFlightControl.h src/SecondaryFlightControl.cpp
        include/Model/SecondaryFlightControlData.h
        include/Model/AircraftHandle.h src/AircraftHandle.cpp
        include/Model/AircraftHandleData.h
        include/Model/Light.h src/Light.cpp
        include/Model/LightData.h
        include/Model/AircraftType.h src/AircraftType.cpp
        include/Model/AircraftInfo.h src/AircraftInfo.cpp
        include/Model/Waypoint.h src/Waypoint.cpp
        include/Model/FlightPlan.h src/FlightPlan.cpp
        include/Model/SimType.h
        include/Model/SimVar.h
        include/Model/SkySearch.h src/SkySearch.cpp
        include/Model/Location.h src/Location.cpp
        include/Model/TimeZoneInfo.h src/TimeZoneInfo.cpp
        include/Model/Enumeration.h src/Enumeration.cpp        
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE
        include/Model
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt6::Core
    PRIVATE
        Sky::Kernel
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
