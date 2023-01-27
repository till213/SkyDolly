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
#include <forward_list>
#include <cstdint>

#include <QSqlDatabase>

#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/FlightDaoIntf.h"
#include <Service/FlightService.h>

struct FlightServicePrivate
{
    FlightServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          flightDao(daoFactory->createFlightDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<FlightDaoIntf> flightDao;
};

// PUBLIC

FlightService::FlightService() noexcept
    : d(std::make_unique<FlightServicePrivate>())
{}

FlightService::FlightService(FlightService &&rhs) noexcept = default;
FlightService &FlightService::operator=(FlightService &&rhs) noexcept = default;
FlightService::~FlightService() = default;

bool FlightService::store(Flight &flight, QSqlDatabase &db) noexcept
{
    bool ok = db.transaction();
    if (ok) {
        ok = d->flightDao->add(flight, db);
        if (ok) {
            ok = db.commit();
            emit flight.flightStored(flight.getId());
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool FlightService::restore(std::int64_t id, QSqlDatabase &db, Flight &flight) noexcept
{
    bool ok = db.transaction();
    if (ok) {
        flight.blockSignals(true);
        ok = d->flightDao->get(id, db, flight);
        flight.blockSignals(false);
        emit flight.flightRestored(flight.getId());
    }
    db.rollback();
    return ok;
}

bool FlightService::deleteById(std::int64_t id, QSqlDatabase &db) noexcept
{
    bool ok = db.transaction();
    if (ok) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        if (flight.getId() == id) {
            flight.clear(true);
        }
        ok = d->flightDao->deleteById(id, db);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool FlightService::updateTitle(Flight &flight, const QString &title, QSqlDatabase &db) noexcept
{
    const bool ok = updateTitle(flight.getId(), title, db);
    if (ok) {
        flight.setTitle(title);
    }
    return ok;
}

bool FlightService::updateTitle(std::int64_t id, const QString &title, QSqlDatabase &db) noexcept
{
    bool ok = db.transaction();
    if (ok) {
        ok = d->flightDao->updateTitle(id, title, db);
        if (ok) {
            ok = db.commit();
            emit Logbook::getInstance().flightTitleOrDescriptionChanged(id);
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool FlightService::updateTitleAndDescription(Flight &flight, const QString &title, const QString &description, QSqlDatabase &db) noexcept
{
    const bool ok = updateTitleAndDescription(flight.getId(), title, description, db);
    if (ok) {
        flight.setTitle(title);
        flight.setDescription(description);
    }
    return ok;
}

bool FlightService::updateTitleAndDescription(std::int64_t id, const QString &title, const QString &description, QSqlDatabase &db) noexcept
{
    bool ok = db.transaction();
    if (ok) {
        ok = d->flightDao->updateTitleAndDescription(id, title, description, db);
        if (ok) {
            ok = db.commit();
            emit Logbook::getInstance().flightTitleOrDescriptionChanged(id);
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool FlightService::updateUserAircraftIndex(Flight &flight, int index, QSqlDatabase &db) noexcept
{
    bool ok = db.transaction();
    if (ok) {
        flight.switchUserAircraftIndex(index);
        ok = d->flightDao->updateUserAircraftIndex(flight.getId(), index, db);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
    }
    return ok;
}
