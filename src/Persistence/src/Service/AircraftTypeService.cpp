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
#include <utility>
#include <vector>

#include <QSqlDatabase>
#include <QStringView>

#include <Model/AircraftType.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/AircraftTypeDaoIntf.h"
#include <Service/AircraftTypeService.h>

class AircraftTypeServicePrivate
{
public:
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

AircraftTypeService::~AircraftTypeService() noexcept
{}

bool AircraftTypeService::getByType(const QString &type, AircraftType &aircraftType) const noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->aircraftTypeDao->getByType(type, aircraftType);
        QSqlDatabase::database().rollback();
    }
    return ok;
}

bool AircraftTypeService::getAll(std::back_insert_iterator<std::vector<AircraftType>> backInsertIterator) const noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->aircraftTypeDao->getAll(backInsertIterator);
        QSqlDatabase::database().rollback();
    }
    return ok;
}

bool AircraftTypeService::exists(const QString &type) const noexcept
{
    bool exists;
    if (QSqlDatabase::database().transaction()) {
        exists = d->aircraftTypeDao->exists(type);
        QSqlDatabase::database().rollback();
    } else {
        exists = false;
    }
    return exists;
}
