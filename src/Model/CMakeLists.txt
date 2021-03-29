add_definitions(-DMODEL_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(Model SHARED "")
else()
    add_library(Model SHARED "")
endif()

target_sources(Model
    PRIVATE
        src/ModelLib.h
        src/World.cpp
        src/World.h
        src/Scenario.cpp
        src/Scenario.h
        src/FlightCondition.cpp
        src/FlightCondition.h
        src/Aircraft.cpp
        src/Aircraft.h
        src/TimeVariableData.cpp
        src/TimeVariableData.h
        src/AircraftData.cpp
        src/AircraftData.h
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
        src/AircraftInfo.cpp
        src/AircraftInfo.h
        src/SimVar.cpp
        src/SimVar.h
        src/SimType.h
        src/SkySearch.cpp
        src/SkySearch.h
        src/CSVConst.h
        src/Export/CSVExport.cpp
        src/Export/CSVExport.h
        src/Import/CSVImport.cpp
        src/Import/CSVImport.h
)

set(Model_LIBS
    Qt${QT_VERSION_MAJOR}::Core Kernel
)
if (APPLE)
    list(APPEND Model_LIBS -lc++)
endif()

target_link_libraries(Model PRIVATE ${Model_LIBS})
set_target_properties(Model PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Model PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(Model PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)
