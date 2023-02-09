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
#include <QSqlError>

#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/FlightData.h>
#include <Model/Aircraft.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/FlightDaoIntf.h"
#include <Service/FlightService.h>

struct FlightServicePrivate
{
    FlightServicePrivate(QString connectionName) noexcept
        : connectionName(connectionName),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite, std::move(connectionName))),
          flightDao(daoFactory->createFlightDao())
    {}

    QString connectionName;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<FlightDaoIntf> flightDao;
};

// PUBLIC

FlightService::FlightService(QString connectionName) noexcept
    : d(std::make_unique<FlightServicePrivate>(std::move(connectionName)))
{}

FlightService::FlightService(FlightService &&rhs) noexcept = default;
FlightService &FlightService::operator=(FlightService &&rhs) noexcept = default;
FlightService::~FlightService() = default;

bool FlightService::store(Flight &flight) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        FlightData &flightData = flight.getFlightData();
        ok = d->flightDao->add(flightData);
        if (ok) {
            ok = db.commit();
            emit flight.flightStored(flight.getId());
        } else {
            db.rollback();
        }
#ifdef DEBUG
    } else {
        qDebug() << "FlightService::store: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool FlightService::exportFlightData(const FlightData &flightData) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->flightDao->exportFlightData(flightData);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
#ifdef DEBUG
    } else {
        qDebug() << "FlightService::exportFlightData: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool FlightService::restore(std::int64_t id, Flight &flight) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        FlightData &flightData = flight.getFlightData();
        ok = d->flightDao->get(id, flightData);
        emit flight.flightRestored(flight.getId());
        db.rollback();
#ifdef DEBUG
    } else {
        qDebug() << "FlightService::restore: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool FlightService::importFlightData(std::int64_t id, FlightData &flightData) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->flightDao->get(id, flightData);
        db.rollback();
#ifdef DEBUG
    } else {
        qDebug() << "FlightService::importFlightData: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool FlightService::deleteById(std::int64_t id) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        if (flight.getId() == id) {
            flight.clear(true);
        }
        ok = d->flightDao->deleteById(id);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
#ifdef DEBUG
    } else {
        qDebug() << "FlightService::deleteById: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool FlightService::updateTitle(Flight &flight, const QString &title) noexcept
{
    const bool ok = updateTitle(flight.getId(), title);
    if (ok) {
        flight.setTitle(title);
    }
    return ok;
}

bool FlightService::updateTitle(std::int64_t id, const QString &title) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->flightDao->updateTitle(id, title);
        if (ok) {
            ok = db.commit();
            emit Logbook::getInstance().flightTitleOrDescriptionChanged(id);
        } else {
            db.rollback();
        }
#ifdef DEBUG
    } else {
        qDebug() << "FlightService::updateTitle: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool FlightService::updateTitleAndDescription(Flight &flight, const QString &title, const QString &description) noexcept
{
    const bool ok = updateTitleAndDescription(flight.getId(), title, description);
    if (ok) {
        flight.setTitle(title);
        flight.setDescription(description);
    }
    return ok;
}

bool FlightService::updateTitleAndDescription(std::int64_t id, const QString &title, const QString &description) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->flightDao->updateTitleAndDescription(id, title, description);
        if (ok) {
            ok = db.commit();
            emit Logbook::getInstance().flightTitleOrDescriptionChanged(id);
        } else {
            db.rollback();
        }
#ifdef DEBUG
    } else {
        qDebug() << "FlightService::updateTitleAndDescription: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool FlightService::updateUserAircraftIndex(Flight &flight, int index) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        flight.switchUserAircraftIndex(index);
        ok = d->flightDao->updateUserAircraftIndex(flight.getId(), index);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
#ifdef DEBUG
    } else {
        qDebug() << "FlightService::updateUserAircraftIndex: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
    }
    return ok;
}
