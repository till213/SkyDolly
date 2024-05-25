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
#include <PersistedEnumerationItem.h>
#include <Service/EnumerationService.h>
#include <Service/LocationService.h>

struct LocationServicePrivate
{
    LocationServicePrivate(QString connectionName) noexcept
        : connectionName(connectionName),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite, std::move(connectionName))),
          locationDao(daoFactory->createLocationDao())
    {}

    QString connectionName;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<LocationDaoIntf> locationDao;
};

// PUBLIC

LocationService::LocationService(QString connectionName) noexcept
    : d {std::make_unique<LocationServicePrivate>(std::move(connectionName))}
{}

LocationService::LocationService(LocationService &&rhs) noexcept = default;
LocationService &LocationService::operator=(LocationService &&rhs) noexcept = default;
LocationService::~LocationService() = default;

bool LocationService::store(Location &location) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->locationDao->add(location);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool LocationService::exportLocation(const Location &location) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->locationDao->exportLocation(location);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool LocationService::storeAll(std::vector<Location> &locations, Mode mode, double distanceKm) noexcept
{
    static const std::int64_t systemLocationTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeSystemSymId).id()};
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    auto it = locations.begin();
    while (it != locations.end() && ok) {
        if (mode != Mode::Insert) {
            const std::vector<Location> neighbours = d->locationDao->getByPosition(it->latitude, it->longitude, distanceKm, &ok);
            if (neighbours.size() == 0) {
                ok = d->locationDao->add(*it);
            } else if (mode == Mode::Update) {                
                // System locations are never updated
                if (neighbours.cbegin()->typeId != systemLocationTypeId) {
                    it->id = neighbours.cbegin()->id;
                    d->locationDao->update(*it);
                }
            }
        } else {
            // Unconditional insert
            ok = d->locationDao->add(*it);
        }
        ++it;
    }
    if (ok) {
        ok = db.commit();
    } else {
        db.rollback();
    }
    return ok;
}

bool LocationService::exportAll(const std::vector<Location> &locations) noexcept
{
    static const std::int64_t systemLocationTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeSystemSymId).id()};
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    auto it = locations.begin();
    while (it != locations.end() && ok) {
        // Unconditional insert
        ok = d->locationDao->exportLocation(*it);
        ++it;
    }
    if (ok) {
        ok = db.commit();
    } else {
        db.rollback();
    }
    return ok;
}

bool LocationService::update(const Location &location) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->locationDao->update(location);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool LocationService::deleteById(std::int64_t id) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->locationDao->deleteById(id);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
    }
    return ok;
}

std::vector<Location> LocationService::getAll(bool *ok) const noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    std::vector<Location> locations;
    bool success = db.transaction();
    if (success) {
        locations = d->locationDao->getAll(&success);
        db.rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return locations;
}

std::vector<Location> LocationService::getSelectedLocations(const LocationSelector &locationSelector, bool *ok) const noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    std::vector<Location> locations;
    bool success = db.transaction();
    if (success) {
        locations = d->locationDao->getSelectedLocations(locationSelector, &success);
        db.rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return locations;
}
