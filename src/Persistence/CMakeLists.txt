add_definitions(-DPERSISTENCE_EXPORT)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(Persistence SHARED "")
else()
    add_library(Persistence SHARED "")
endif()

target_sources(Persistence
    PRIVATE
        src/PersistenceLib.h
        src/ConnectionManager.cpp
        src/ConnectionManager.h
        src/Metadata.h
        src/Dao/DatabaseDaoIntf.h
        src/Dao/FlightDaoIntf.h
        src/Dao/AircraftDaoIntf.h
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
        src/Dao/SQLite/SQLiteFlightDao.cpp
        src/Dao/SQLite/SQLiteFlightDao.h
        src/Dao/SQLite/SQLiteAircraftDao.cpp
        src/Dao/SQLite/SQLiteAircraftDao.h
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
        src/Dao/SQLite/Migration/migr-ex-ante.sql
        src/Dao/SQLite/Migration/migr-ddl.sql
        src/Dao/SQLite/Migration/migr-ex-post.sql
        src/Dao/SQLite/Migration/Migration.qrc
        src/Service/FlightService.cpp
        src/Service/FlightService.h
        src/Service/AircraftService.cpp
        src/Service/AircraftService.h
        src/Service/DatabaseService.cpp
        src/Service/DatabaseService.h
        src/Service/CSVService.cpp
        src/Service/CSVService.h
        src/CSVConst.cpp
        src/CSVConst.h
        src/Export/CSVExport.cpp
        src/Export/CSVExport.h
        src/Import/CSVImport.cpp
        src/Import/CSVImport.h
)

set(PERSISTENCE_LIBS
    Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Sql Kernel Model SkyConnect
)
if (APPLE)
    list(APPEND PERSISTENCE_LIBS -lc++)
endif()

target_link_libraries(Persistence PRIVATE ${PERSISTENCE_LIBS})
set_target_properties(Persistence PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(Persistence PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(Persistence PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}"
)
