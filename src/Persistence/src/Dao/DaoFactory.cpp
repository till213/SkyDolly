/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include "DaoFactory.h"

class DaoFactoryPrivate
{
public:
    DaoFactoryPrivate(DaoFactory::DbType theDbType)
        : dbType(theDbType)
    {}

    DaoFactory::DbType dbType;
};

// PUBLIC

DaoFactory::DaoFactory(DbType dbType)
    : d(std::make_unique<DaoFactoryPrivate>(dbType))
{}

DaoFactory::~DaoFactory()
{}

std::unique_ptr<DatabaseDaoIntf> DaoFactory::createDatabaseDao() noexcept
{
    std::unique_ptr<DatabaseDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteDatabaseDao>();
        break;
    }
    return dao;
}

std::unique_ptr<LogbookDaoIntf> DaoFactory::createLogbookDao() noexcept
{
    std::unique_ptr<SQLiteLogbookDao> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteLogbookDao>();
        break;
    }
    return dao;
}

std::unique_ptr<FlightDaoIntf> DaoFactory::createFlightDao() noexcept
{
    std::unique_ptr<SQLiteFlightDao> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteFlightDao>();
        break;
    }
    return dao;
}

std::unique_ptr<AircraftDaoIntf> DaoFactory::createAircraftDao() noexcept
{
    std::unique_ptr<SQLiteAircraftDao> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteAircraftDao>();
        break;
    }
    return dao;
}

std::unique_ptr<AircraftTypeDaoIntf> DaoFactory::createAircraftTypeDao() noexcept
{
    std::unique_ptr<SQLiteAircraftTypeDao> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteAircraftTypeDao>();
        break;
    }
    return dao;
}

std::unique_ptr<PositionDaoIntf> DaoFactory::createPositionDao() noexcept
{
    std::unique_ptr<PositionDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLitePositionDao>();
        break;
    }
    return dao;
}

std::unique_ptr<EngineDaoIntf> DaoFactory::createEngineDao() noexcept
{
    std::unique_ptr<EngineDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteEngineDao>();
        break;
    }
    return dao;
}

std::unique_ptr<PrimaryFlightControlDaoIntf> DaoFactory::createPrimaryFlightControlDao() noexcept
{
    std::unique_ptr<PrimaryFlightControlDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLitePrimaryFlightControlDao>();
        break;
    }
    return dao;
};

std::unique_ptr<SecondaryFlightControlDaoIntf> DaoFactory::createSecondaryFlightControlDao() noexcept
{
    std::unique_ptr<SecondaryFlightControlDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteSecondaryFlightControlDao>();
        break;
    }
    return dao;
}

std::unique_ptr<HandleDaoIntf> DaoFactory::createHandleDao() noexcept
{
    std::unique_ptr<HandleDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteHandleDao>();
        break;
    }
    return dao;
}

std::unique_ptr<LightDaoIntf> DaoFactory::createLightDao() noexcept
{
    std::unique_ptr<LightDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteLightDao>();
        break;
    }
    return dao;
}

std::unique_ptr<WaypointDaoIntf> DaoFactory::createFlightPlanDao() noexcept
{
    std::unique_ptr<WaypointDaoIntf> dao {nullptr};
    switch (d->dbType) {
    case DbType::SQLite:
        dao = std::make_unique<SQLiteWaypointDao>();
        break;
    }
    return dao;
}
