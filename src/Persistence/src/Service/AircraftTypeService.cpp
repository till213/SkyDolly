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
    AircraftTypeServicePrivate(const QSqlDatabase &db) noexcept
        : db(db),
          daoFactory(std::make_unique<DaoFactory>()),
          aircraftTypeDao(daoFactory->createAircraftTypeDao(db))
    {}

    QSqlDatabase db;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftTypeDaoIntf> aircraftTypeDao;
};

// PUBLIC

AircraftTypeService::AircraftTypeService(const QSqlDatabase &db) noexcept
    : d(std::make_unique<AircraftTypeServicePrivate>(db))
{}

AircraftTypeService::AircraftTypeService(AircraftTypeService &&rhs) noexcept = default;
AircraftTypeService &AircraftTypeService::operator=(AircraftTypeService &&rhs) noexcept = default;
AircraftTypeService::~AircraftTypeService() = default;

AircraftType AircraftTypeService::getByType(const QString &type, bool *ok) const noexcept
{
    AircraftType aircraftType;
    bool success = d->db.transaction();
    if (success) {
        aircraftType = d->aircraftTypeDao->getByType(type, &success);
        d->db.rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftType;
}

std::vector<AircraftType> AircraftTypeService::getAll(bool *ok) const noexcept
{
    std::vector<AircraftType> aircraftTypes;
    bool success = d->db.transaction();
    if (success) {
        aircraftTypes = d->aircraftTypeDao->getAll(&success);
        d->db.rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftTypes;
}

bool AircraftTypeService::exists(const QString &type) const noexcept
{
    bool exists {false};
    if (d->db.transaction()) {
        exists = d->aircraftTypeDao->exists(type);
        d->db.rollback();
    }
    return exists;
}
