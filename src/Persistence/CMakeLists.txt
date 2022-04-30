set(LIBRARY_NAME "Persistence")

add_definitions(-DPERSISTENCE_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(SD::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
    PRIVATE
        src/PersistenceLib.h
        src/ConnectionManager.h src/ConnectionManager.cpp
        src/Metadata.h
        src/Dao/DatabaseDaoIntf.h
        src/Dao/LogbookDaoIntf.h
        src/Dao/FlightDaoIntf.h
        src/Dao/FlightSelector.h src/Dao/FlightSelector.cpp
        src/Dao/AircraftDaoIntf.h
        src/Dao/AircraftTypeDaoIntf.h
        src/Dao/PositionDaoIntf.h
        src/Dao/EngineDaoIntf.h
        src/Dao/PrimaryFlightControlDaoIntf.h
        src/Dao/SecondaryFlightControlDaoIntf.h
        src/Dao/HandleDaoIntf.h
        src/Dao/LightDaoIntf.h
        src/Dao/WaypointDaoIntf.h
        src/Dao/DaoFactory.h src/Dao/DaoFactory.cpp
        src/Dao/SQLite/SQLiteDatabaseDao.h src/Dao/SQLite/SQLiteDatabaseDao.cpp
        src/Dao/SQLite/SQLiteLogbookDao.h src/Dao/SQLite/SQLiteLogbookDao.cpp
        src/Dao/SQLite/SQLiteFlightDao.h src/Dao/SQLite/SQLiteFlightDao.cpp
        src/Dao/SQLite/SQLiteAircraftDao.h src/Dao/SQLite/SQLiteAircraftDao.cpp
        src/Dao/SQLite/SQLiteAircraftTypeDao.h src/Dao/SQLite/SQLiteAircraftTypeDao.cpp
        src/Dao/SQLite/SQLitePositionDao.h src/Dao/SQLite/SQLitePositionDao.cpp
        src/Dao/SQLite/SQLiteEngineDao.h src/Dao/SQLite/SQLiteEngineDao.cpp
        src/Dao/SQLite/SQLitePrimaryFlightControlDao.h src/Dao/SQLite/SQLitePrimaryFlightControlDao.cpp
        src/Dao/SQLite/SQLiteSecondaryFlightControlDao.h src/Dao/SQLite/SQLiteSecondaryFlightControlDao.cpp
        src/Dao/SQLite/SQLiteHandleDao.h src/Dao/SQLite/SQLiteHandleDao.cpp
        src/Dao/SQLite/SQLiteLightDao.h src/Dao/SQLite/SQLiteLightDao.cpp
        src/Dao/SQLite/SQLiteWaypointDao.h src/Dao/SQLite/SQLiteWaypointDao.cpp
        src/Dao/SQLite/SqlMigration.h src/Dao/SQLite/SqlMigration.cpp
        src/Dao/SQLite/SqlMigrationStep.h src/Dao/SQLite/SqlMigrationStep.cpp
        src/Dao/SQLite/Migration/migr.sql
        src/Dao/SQLite/Migration/Migration.qrc
        src/Service/LogbookService.h src/Service/LogbookService.cpp
        src/Service/FlightService.h src/Service/FlightService.cpp
        src/Service/AircraftService.h src/Service/AircraftService.cpp
        src/Service/AircraftTypeService.h src/Service/AircraftTypeService.cpp
        src/Service/DatabaseService.h src/Service/DatabaseService.cpp
)

set(LIBRARY_DEPENDENCIES
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    SD::Kernel
    SD::Model
    SD::SkyConnect
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
