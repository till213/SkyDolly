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
#include <cstdint>
#include <vector>

#include <QSqlDatabase>

#include <Model/Location.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/LocationDaoIntf.h"
#include <PersistenceManager.h>
#include <LocationSelector.h>
#include <Service/LocationService.h>

struct LocationServicePrivate
{
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
{}

LocationService::LocationService(LocationService &&rhs) noexcept = default;
LocationService &LocationService::operator=(LocationService &&rhs) noexcept = default;
LocationService::~LocationService() = default;

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

bool LocationService::storeAll(std::vector<Location> &locations, Mode mode) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    auto it = locations.begin();
    while (it != locations.end() && ok) {
        if (mode != Mode::Insert) {
            const std::vector<Location> neighbours = d->locationDao->getByPosition(it->latitude, it->longitude, 0.0, &ok);
            if (neighbours.size() == 0) {
                ok = d->locationDao->add(*it);
            } else if (mode == Mode::Update) {
                it->id = neighbours.cbegin()->id;
                d->locationDao->update(*it);
            }
        } else {
            // Unconditional insert
            ok = d->locationDao->add(*it);
        }
        ++it;
    }
    if (ok) {
        ok = QSqlDatabase::database().commit();
    } else {
        QSqlDatabase::database().rollback();
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

std::vector<Location> LocationService::getAll(bool *ok) const noexcept
{
    std::vector<Location> locations;
    bool success = QSqlDatabase::database().transaction();
    if (success) {
        locations = d->locationDao->getAll(&success);
        QSqlDatabase::database().rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return locations;
}

std::vector<Location> LocationService::getSelectedLocations(const LocationSelector &locationSelector, bool *ok) const noexcept
{
    std::vector<Location> locations;
    bool success = QSqlDatabase::database().transaction();
    if (success) {
        locations = d->locationDao->getSelectedLocations(locationSelector, &success);
        QSqlDatabase::database().rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return locations;
}
