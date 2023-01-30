/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <memory>

#include <QString>
#include <QSqlDatabase>

#include "SQLite/SQLiteDatabaseDao.h"
#include "SQLite/SQLiteLogbookDao.h"
#include "SQLite/SQLiteFlightDao.h"
#include "SQLite/SQLiteAircraftDao.h"
#include "SQLite/SQLiteAircraftTypeDao.h"
#include "SQLite/SQLitePositionDao.h"
#include "SQLite/SQLiteEngineDao.h"
#include "SQLite/SQLitePrimaryFlightControlDao.h"
#include "SQLite/SQLiteSecondaryFlightControlDao.h"
#include "SQLite/SQLiteHandleDao.h"
#include "SQLite/SQLiteLightDao.h"
#include "SQLite/SQLiteWaypointDao.h"
#include "SQLite/SQLiteLocationDao.h"
#include "SQLite/SQLiteEnumerationDao.h"
#include "FlightDaoIntf.h"
#include "AircraftDaoIntf.h"
#include "AircraftTypeDaoIntf.h"
#include "PositionDaoIntf.h"
#include "EngineDaoIntf.h"
#include "PrimaryFlightControlDaoIntf.h"
#include "SecondaryFlightControlDaoIntf.h"
#include "HandleDaoIntf.h"
#include "LightDaoIntf.h"
#include "WaypointDaoIntf.h"
#include "LocationDaoIntf.h"
#include "EnumerationDaoIntf.h"
#include "DaoFactory.h"

// PUBLIC

DaoFactory::DaoFactory(DaoFactory &&rhs) noexcept = default;
DaoFactory &DaoFactory::operator=(DaoFactory &&rhs) noexcept = default;
DaoFactory::~DaoFactory() = default;

std::unique_ptr<DatabaseDaoIntf> DaoFactory::createDatabaseDao(DbType dbType) noexcept
{
    std::unique_ptr<DatabaseDaoIntf> dao {nullptr};
    switch (dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteDatabaseDao>();
        break;
    }
    return dao;
}

std::unique_ptr<LogbookDaoIntf> DaoFactory::createLogbookDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<SQLiteLogbookDao> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteLogbookDao>(db);
    }
    return dao;
}

std::unique_ptr<FlightDaoIntf> DaoFactory::createFlightDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<SQLiteFlightDao> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteFlightDao>(db);
    }
    return dao;
}

std::unique_ptr<AircraftDaoIntf> DaoFactory::createAircraftDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<SQLiteAircraftDao> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteAircraftDao>(db);
    }
    return dao;
}

std::unique_ptr<AircraftTypeDaoIntf> DaoFactory::createAircraftTypeDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<SQLiteAircraftTypeDao> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteAircraftTypeDao>(db);
    }
    return dao;
}

std::unique_ptr<PositionDaoIntf> DaoFactory::createPositionDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<PositionDaoIntf> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLitePositionDao>(db);
    }
    return dao;
}

std::unique_ptr<EngineDaoIntf> DaoFactory::createEngineDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<EngineDaoIntf> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteEngineDao>(db);
    }
    return dao;
}

std::unique_ptr<PrimaryFlightControlDaoIntf> DaoFactory::createPrimaryFlightControlDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<PrimaryFlightControlDaoIntf> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLitePrimaryFlightControlDao>(db);
    }
    return dao;
};

std::unique_ptr<SecondaryFlightControlDaoIntf> DaoFactory::createSecondaryFlightControlDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<SecondaryFlightControlDaoIntf> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteSecondaryFlightControlDao>(db);
    }
    return dao;
}

std::unique_ptr<HandleDaoIntf> DaoFactory::createHandleDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<HandleDaoIntf> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteHandleDao>(db);
    }
    return dao;
}

std::unique_ptr<LightDaoIntf> DaoFactory::createLightDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<LightDaoIntf> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteLightDao>(db);
    }
    return dao;
}

std::unique_ptr<WaypointDaoIntf> DaoFactory::createFlightPlanDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<WaypointDaoIntf> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteWaypointDao>(db);
    }
    return dao;
}

std::unique_ptr<LocationDaoIntf> DaoFactory::createLocationDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<LocationDaoIntf> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteLocationDao>(db);
    }
    return dao;
}


std::unique_ptr<EnumerationDaoIntf> DaoFactory::createEnumerationDao(const QSqlDatabase &db) noexcept
{
    std::unique_ptr<EnumerationDaoIntf> dao {nullptr};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteEnumerationDao>(db);
    }
    return dao;
}
