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
#include <cstdint>
#include <iterator>

#include <QSqlDatabase>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/Location.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/LocationDaoIntf.h"
#include <LocationSelector.h>
#include <Service/LocationService.h>

class LocationServicePrivate
{
public:
    LocationServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          locationDao(daoFactory->createLocationDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<LocationDaoIntf> locationDao;
};

// PUBLIC

LocationService::LocationService() noexcept
    : d(std::make_unique<LocationServicePrivate>())
{
#ifdef DEBUG
    qDebug() << "LocationService::LocationService: CREATED.";
#endif
}

LocationService::~LocationService() noexcept
{
#ifdef DEBUG
    qDebug() << "LocationService::~LocationService: DELETED.";
#endif
}

bool LocationService::store(Location &location) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->locationDao->add(location);
        if (ok) {
            ok = QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool LocationService::update(const Location &location) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->locationDao->update(location);
        if (ok) {
            ok = QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool LocationService::deleteById(std::int64_t id) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->locationDao->deleteById(id);
        if (ok) {
            ok = QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool LocationService::getAll(std::back_insert_iterator<std::vector<Location>> backInsertIterator) const noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->locationDao->getAll(backInsertIterator);
        QSqlDatabase::database().rollback();
    }
    return ok;
}

bool LocationService::getSelectedLocations(const LocationSelector &locationSelector, std::back_insert_iterator<std::vector<Location>> backInsertIterator) const noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->locationDao->getSelectedLocations(locationSelector, backInsertIterator);
        QSqlDatabase::database().rollback();
    }
    return ok;
}
