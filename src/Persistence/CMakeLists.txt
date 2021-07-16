set(LIBRARY_NAME "Persistence")

add_definitions(-DPERSISTENCE_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED "")
else()
    add_library(${LIBRARY_NAME} SHARED "")
endif()

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/PersistenceLib.h
        src/ConnectionManager.cpp
        src/ConnectionManager.h
        src/Metadata.h
        src/Dao/DatabaseDaoIntf.h
        src/Dao/LogbookDaoIntf.h
        src/Dao/FlightDaoIntf.h
        src/Dao/FlightSelector.cpp
        src/Dao/FlightSelector.h
        src/Dao/AircraftDaoIntf.h
        src/Dao/AircraftTypeDaoIntf.h
        src/Dao/PositionDaoIntf.h
        src/Dao/EngineDaoIntf.h
        src/Dao/PrimaryFlightControlDaoIntf.h
        src/Dao/SecondaryFlightControlDaoIntf.h
        src/Dao/HandleDaoIntf.h
        src/Dao/LightDaoIntf.h
        src/Dao/WaypointDaoIntf.h
        src/Dao/DaoFactory.cpp
        src/Dao/DaoFactory.h
        src/Dao/SQLite/SQLiteDatabaseDao.cpp
        src/Dao/SQLite/SQLiteDatabaseDao.h
        src/Dao/SQLite/SQLiteLogbookDao.cpp
        src/Dao/SQLite/SQLiteLogbookDao.h
        src/Dao/SQLite/SQLiteFlightDao.cpp
        src/Dao/SQLite/SQLiteFlightDao.h
        src/Dao/SQLite/SQLiteAircraftDao.cpp
        src/Dao/SQLite/SQLiteAircraftDao.h
        src/Dao/SQLite/SQLiteAircraftTypeDao.cpp
        src/Dao/SQLite/SQLiteAircraftTypeDao.h
        src/Dao/SQLite/SQLitePositionDao.cpp
        src/Dao/SQLite/SQLitePositionDao.h
        src/Dao/SQLite/SQLiteEngineDao.cpp
        src/Dao/SQLite/SQLiteEngineDao.h
        src/Dao/SQLite/SQLitePrimaryFlightControlDao.cpp
        src/Dao/SQLite/SQLitePrimaryFlightControlDao.h
        src/Dao/SQLite/SQLiteSecondaryFlightControlDao.cpp
        src/Dao/SQLite/SQLiteSecondaryFlightControlDao.h
        src/Dao/SQLite/SQLiteHandleDao.cpp
        src/Dao/SQLite/SQLiteHandleDao.h
        src/Dao/SQLite/SQLiteLightDao.cpp
        src/Dao/SQLite/SQLiteLightDao.h
        src/Dao/SQLite/SQLiteWaypointDao.cpp
        src/Dao/SQLite/SQLiteWaypointDao.h
        src/Dao/SQLite/SqlMigration.cpp
        src/Dao/SQLite/SqlMigration.h
        src/Dao/SQLite/SqlMigrationStep.cpp
        src/Dao/SQLite/SqlMigrationStep.h
        src/Dao/SQLite/Migration/migr.sql
        src/Dao/SQLite/Migration/Migration.qrc
        src/Service/LogbookService.cpp
        src/Service/LogbookService.h
        src/Service/FlightService.cpp
        src/Service/FlightService.h
        src/Service/AircraftService.cpp
        src/Service/AircraftService.h
        src/Service/AircraftTypeService.cpp
        src/Service/AircraftTypeService.h
        src/Service/DatabaseService.cpp
        src/Service/DatabaseService.h
        src/CSVConst.h
)

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Kernel Model SkyConnect
)
if (${PLATFORM_IS_MACOS})
    list(APPEND LIBRARY_DEPENDENCIES -lc++)
endif()

target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_DEPENDENCIES})
set_target_properties(${LIBRARY_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${LIBRARY_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(${LIBRARY_NAME} PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)
