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

std::unique_ptr<LogbookDaoIntf> DaoFactory::createLogbookDao(const QString &connectionName) noexcept
{
    std::unique_ptr<SQLiteLogbookDao> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteLogbookDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<FlightDaoIntf> DaoFactory::createFlightDao(const QString &connectionName) noexcept
{
    std::unique_ptr<SQLiteFlightDao> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteFlightDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<AircraftDaoIntf> DaoFactory::createAircraftDao(const QString &connectionName) noexcept
{
    std::unique_ptr<SQLiteAircraftDao> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteAircraftDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<AircraftTypeDaoIntf> DaoFactory::createAircraftTypeDao(const QString &connectionName) noexcept
{
    std::unique_ptr<SQLiteAircraftTypeDao> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteAircraftTypeDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<PositionDaoIntf> DaoFactory::createPositionDao(const QString &connectionName) noexcept
{
    std::unique_ptr<PositionDaoIntf> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLitePositionDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<EngineDaoIntf> DaoFactory::createEngineDao(const QString &connectionName) noexcept
{
    std::unique_ptr<EngineDaoIntf> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteEngineDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<PrimaryFlightControlDaoIntf> DaoFactory::createPrimaryFlightControlDao(const QString &connectionName) noexcept
{
    std::unique_ptr<PrimaryFlightControlDaoIntf> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLitePrimaryFlightControlDao>(connectionName);
    }
    return dao;
};

std::unique_ptr<SecondaryFlightControlDaoIntf> DaoFactory::createSecondaryFlightControlDao(const QString &connectionName) noexcept
{
    std::unique_ptr<SecondaryFlightControlDaoIntf> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteSecondaryFlightControlDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<HandleDaoIntf> DaoFactory::createHandleDao(const QString &connectionName) noexcept
{
    std::unique_ptr<HandleDaoIntf> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteHandleDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<LightDaoIntf> DaoFactory::createLightDao(const QString &connectionName) noexcept
{
    std::unique_ptr<LightDaoIntf> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteLightDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<WaypointDaoIntf> DaoFactory::createFlightPlanDao(const QString &connectionName) noexcept
{
    std::unique_ptr<WaypointDaoIntf> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteWaypointDao>(connectionName);
    }
    return dao;
}

std::unique_ptr<LocationDaoIntf> DaoFactory::createLocationDao(const QString &connectionName) noexcept
{
    std::unique_ptr<LocationDaoIntf> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteLocationDao>(connectionName);
    }
    return dao;
}


std::unique_ptr<EnumerationDaoIntf> DaoFactory::createEnumerationDao(const QString &connectionName) noexcept
{
    std::unique_ptr<EnumerationDaoIntf> dao {nullptr};
    QSqlDatabase db {QSqlDatabase::database(connectionName)};
    if (db.driverName() == SQLiteDriver) {
        dao = std::make_unique<SQLiteEnumerationDao>(connectionName);
    }
    return dao;
}
