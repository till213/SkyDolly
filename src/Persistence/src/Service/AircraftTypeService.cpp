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
#include <vector>

#include <QSqlDatabase>
#include <QStringView>

#include <Model/AircraftType.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/AircraftTypeDaoIntf.h"
#include <Service/AircraftTypeService.h>

struct AircraftTypeServicePrivate
{
    AircraftTypeServicePrivate(QString connectionName) noexcept
        : connectionName(connectionName),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite, std::move(connectionName))),
          aircraftTypeDao(daoFactory->createAircraftTypeDao())
    {}

    QString connectionName;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftTypeDaoIntf> aircraftTypeDao;
};

// PUBLIC

AircraftTypeService::AircraftTypeService(QString connectionName) noexcept
    : d {std::make_unique<AircraftTypeServicePrivate>(std::move(connectionName))}
{}

AircraftTypeService::AircraftTypeService(AircraftTypeService &&rhs) noexcept = default;
AircraftTypeService &AircraftTypeService::operator=(AircraftTypeService &&rhs) noexcept = default;
AircraftTypeService::~AircraftTypeService() = default;

AircraftType AircraftTypeService::getByType(const QString &type, bool *ok) const noexcept
{
    AircraftType aircraftType;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool success = db.transaction();
    if (success) {
        aircraftType = d->aircraftTypeDao->getByType(type, &success);
        db.rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftType;
}

std::vector<AircraftType> AircraftTypeService::getAll(bool *ok) const noexcept
{
    std::vector<AircraftType> aircraftTypes;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool success = db.transaction();
    if (success) {
        aircraftTypes = d->aircraftTypeDao->getAll(&success);
        db.rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftTypes;
}

bool AircraftTypeService::exists(const QString &type) const noexcept
{
    bool exists {false};
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    if (db.transaction()) {
        exists = d->aircraftTypeDao->exists(type);
        db.rollback();
    }
    return exists;
}
