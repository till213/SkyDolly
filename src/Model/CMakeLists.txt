set(LIBRARY_NAME "Model")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        MODEL_EXPORT
)
target_sources(${LIBRARY_NAME}
    PRIVATE
        include/Model/ModelLib.h
        include/Model/Data.h src/Data.cpp
        include/Model/Logbook.h src/Logbook.cpp
        include/Model/AbstractComponent.h
        include/Model/Flight.h src/Flight.cpp
        include/Model/FlightSummary.h src/FlightSummary.cpp
        include/Model/FlightDate.h src/FlightDate.cpp
        include/Model/FlightCondition.h src/FlightCondition.cpp
        include/Model/Aircraft.h src/Aircraft.cpp
        include/Model/TimeVariableData.h src/TimeVariableData.cpp
        include/Model/Position.h src/Position.cpp
        include/Model/PositionData.h src/PositionData.cpp
        include/Model/InitialPosition.h src/InitialPosition.cpp
        include/Model/Engine.h src/Engine.cpp
        include/Model/EngineData.h src/EngineData.cpp
        include/Model/PrimaryFlightControl.h src/PrimaryFlightControl.cpp
        include/Model/PrimaryFlightControlData.h src/PrimaryFlightControlData.cpp
        include/Model/SecondaryFlightControl.h src/SecondaryFlightControl.cpp
        include/Model/SecondaryFlightControlData.h src/SecondaryFlightControlData.cpp
        include/Model/AircraftHandle.h src/AircraftHandle.cpp
        include/Model/AircraftHandleData.h src/AircraftHandleData.cpp
        include/Model/Light.h src/Light.cpp
        include/Model/LightData.h src/LightData.cpp
        include/Model/AircraftType.h src/AircraftType.cpp
        include/Model/AircraftInfo.h src/AircraftInfo.cpp
        include/Model/Waypoint.h src/Waypoint.cpp
        include/Model/FlightPlan.h src/FlightPlan.cpp
        include/Model/SimType.h
        include/Model/SimVar.h
        include/Model/SkySearch.h src/SkySearch.cpp
        include/Model/Location.h src/Location.cpp
        include/Model/Enumeration.h src/Enumeration.cpp
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        include/Model
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Sql
    PRIVATE
        Sky::Kernel
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
