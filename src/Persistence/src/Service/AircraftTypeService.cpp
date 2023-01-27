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
    AircraftTypeServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          aircraftTypeDao(daoFactory->createAircraftTypeDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftTypeDaoIntf> aircraftTypeDao;
};

// PUBLIC

AircraftTypeService::AircraftTypeService() noexcept
    : d(std::make_unique<AircraftTypeServicePrivate>())
{}

AircraftTypeService::AircraftTypeService(AircraftTypeService &&rhs) noexcept = default;
AircraftTypeService &AircraftTypeService::operator=(AircraftTypeService &&rhs) noexcept = default;
AircraftTypeService::~AircraftTypeService() = default;

AircraftType AircraftTypeService::getByType(const QString &type, QSqlDatabase &db, bool *ok) const noexcept
{
    AircraftType aircraftType;
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

std::vector<AircraftType> AircraftTypeService::getAll(QSqlDatabase &db, bool *ok) const noexcept
{
    std::vector<AircraftType> aircraftTypes;
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

bool AircraftTypeService::exists(const QString &type, QSqlDatabase &db) const noexcept
{
    bool exists {false};
    if (db.transaction()) {
        exists = d->aircraftTypeDao->exists(type);
        db.rollback();
    }
    return exists;
}
