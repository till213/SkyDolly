/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <utility>

#include "SQLite/SQLiteDatabaseDao.h"
#include "SQLite/SQLiteLogbookDao.h"
#include "SQLite/SQLiteFlightDao.h"
#include "SQLite/SQLiteAircraftDao.h"
#include "SQLite/SQLiteAircraftTypeDao.h"
#include "SQLite/SQLitePositionDao.h"
#include "SQLite/SQLiteAttitudeDao.h"
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
#include "AttitudeDaoIntf.h"
#include "EngineDaoIntf.h"
#include "PrimaryFlightControlDaoIntf.h"
#include "SecondaryFlightControlDaoIntf.h"
#include "HandleDaoIntf.h"
#include "LightDaoIntf.h"
#include "WaypointDaoIntf.h"
#include "LocationDaoIntf.h"
#include "EnumerationDaoIntf.h"
#include "DaoFactory.h"

struct DaoFactoryPrivate
{
    DaoFactoryPrivate(DaoFactory::DbType dbType, QString connectionName)
        : dbType(dbType),
          connectionName(std::move(connectionName))
    {}

    DaoFactory::DbType dbType;
    QString connectionName;
};

// PUBLIC

DaoFactory::DaoFactory(DbType dbType, QString connectionName)
    : d {std::make_unique<DaoFactoryPrivate>(dbType, std::move(connectionName))}
{}

DaoFactory::DaoFactory(DaoFactory &&rhs) noexcept = default;
DaoFactory &DaoFactory::operator=(DaoFactory &&rhs) noexcept = default;
DaoFactory::~DaoFactory() = default;

std::unique_ptr<DatabaseDaoIntf> DaoFactory::createDatabaseDao() noexcept
{
    std::unique_ptr<DatabaseDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteDatabaseDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<LogbookDaoIntf> DaoFactory::createLogbookDao() noexcept
{
    std::unique_ptr<SQLiteLogbookDao> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteLogbookDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<FlightDaoIntf> DaoFactory::createFlightDao() noexcept
{
    std::unique_ptr<SQLiteFlightDao> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteFlightDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<AircraftDaoIntf> DaoFactory::createAircraftDao() noexcept
{
    std::unique_ptr<SQLiteAircraftDao> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteAircraftDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<AircraftTypeDaoIntf> DaoFactory::createAircraftTypeDao() noexcept
{
    std::unique_ptr<SQLiteAircraftTypeDao> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteAircraftTypeDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<PositionDaoIntf> DaoFactory::createPositionDao() noexcept
{
    std::unique_ptr<PositionDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLitePositionDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<AttitudeDaoIntf> DaoFactory::createAttitudeDao() noexcept
{
    std::unique_ptr<AttitudeDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteAttitudeDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<EngineDaoIntf> DaoFactory::createEngineDao() noexcept
{
    std::unique_ptr<EngineDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteEngineDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<PrimaryFlightControlDaoIntf> DaoFactory::createPrimaryFlightControlDao() noexcept
{
    std::unique_ptr<PrimaryFlightControlDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLitePrimaryFlightControlDao>(d->connectionName);
        break;
    }
    return dao;
};

std::unique_ptr<SecondaryFlightControlDaoIntf> DaoFactory::createSecondaryFlightControlDao() noexcept
{
    std::unique_ptr<SecondaryFlightControlDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteSecondaryFlightControlDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<HandleDaoIntf> DaoFactory::createHandleDao() noexcept
{
    std::unique_ptr<HandleDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteHandleDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<LightDaoIntf> DaoFactory::createLightDao() noexcept
{
    std::unique_ptr<LightDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteLightDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<WaypointDaoIntf> DaoFactory::createFlightPlanDao() noexcept
{
    std::unique_ptr<WaypointDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteWaypointDao>(d->connectionName);
        break;
    }
    return dao;
}

std::unique_ptr<LocationDaoIntf> DaoFactory::createLocationDao() noexcept
{
    std::unique_ptr<LocationDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteLocationDao>(d->connectionName);
        break;
    }
    return dao;
}


std::unique_ptr<EnumerationDaoIntf> DaoFactory::createEnumerationDao() noexcept
{
    std::unique_ptr<EnumerationDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteEnumerationDao>(d->connectionName);
        break;
    }
    return dao;
}
