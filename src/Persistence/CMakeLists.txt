set(LIBRARY_NAME "Persistence")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_library(${LIBRARY_NAME} SHARED)
else()
    add_library(${LIBRARY_NAME} SHARED)
endif()
add_library(Sky::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

target_compile_definitions(${LIBRARY_NAME}
    PRIVATE
        PERSISTENCE_EXPORT
)
target_sources(${LIBRARY_NAME}
    PRIVATE
        include/Persistence/PersistenceLib.h
        include/Persistence/LogbookManager.h src/LogbookManager.cpp
        include/Persistence/FlightSelector.h src/FlightSelector.cpp
        include/Persistence/LocationSelector.h src/LocationSelector.cpp
        include/Persistence/Metadata.h
        include/Persistence/PersistedEnumerationItem.h src/PersistedEnumerationItem.cpp
        src/Dao/DatabaseDaoIntf.h
        src/Dao/LogbookDaoIntf.h
        src/Dao/FlightDaoIntf.h
        src/Dao/AircraftDaoIntf.h
        src/Dao/AircraftTypeDaoIntf.h
        src/Dao/PositionDaoIntf.h
        src/Dao/EngineDaoIntf.h
        src/Dao/PrimaryFlightControlDaoIntf.h
        src/Dao/SecondaryFlightControlDaoIntf.h
        src/Dao/HandleDaoIntf.h
        src/Dao/LightDaoIntf.h
        src/Dao/WaypointDaoIntf.h
        src/Dao/LocationDaoIntf.h
        src/Dao/EnumerationDaoIntf.h
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
        src/Dao/SQLite/SQLiteLocationDao.h src/Dao/SQLite/SQLiteLocationDao.cpp
        src/Dao/SQLite/SQLiteEnumerationDao.h src/Dao/SQLite/SQLiteEnumerationDao.cpp
        src/Dao/SQLite/SqlMigration.h src/Dao/SQLite/SqlMigration.cpp
        src/Dao/SQLite/SqlMigrationStep.h src/Dao/SQLite/SqlMigrationStep.cpp
        include/Persistence/Service/LogbookService.h src/Service/LogbookService.cpp
        include/Persistence/Service/FlightService.h src/Service/FlightService.cpp
        include/Persistence/Service/AircraftService.h src/Service/AircraftService.cpp
        include/Persistence/Service/AircraftTypeService.h src/Service/AircraftTypeService.cpp
        include/Persistence/Service/DatabaseService.h src/Service/DatabaseService.cpp
        include/Persistence/Service/LocationService.h src/Service/LocationService.cpp
        include/Persistence/Service/EnumerationService.h src/Service/EnumerationService.cpp
        src/Dao/SQLite/migr/LogbookMigration.sql
        src/Dao/SQLite/migr/LocationMigration.sql
        src/Dao/SQLite/migr/Migration.qrc
        res/migr/Locations.csv
        res/migr/Locations.ods
        res/migr/README.md
)
target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        include/Persistence
)
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Sql
    PRIVATE
        Sky::Kernel
        Sky::Model
)
set_target_properties(${LIBRARY_NAME}
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)
