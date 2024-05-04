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

bool FlightService::storeFlight(Flight &flight) noexcept
{
    FlightData &flightData = flight.getFlightData();
    const bool ok = storeFlightData(flightData);
    emit flight.flightStored(ok);
    return ok;
}

bool FlightService::storeFlightData(FlightData &flightData) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->flightDao->add(flightData);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
#ifdef DEBUG
    } else {
        qDebug() << "FlightService::storeFlightData: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
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

bool FlightService::restoreFlight(std::int64_t id, Flight &flight) noexcept
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

bool FlightService::importFlightData(std::int64_t id, FlightData &flightData) const noexcept
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

bool FlightService::deleteById(std::int64_t id) const noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        auto &flight = Logbook::getInstance().getCurrentFlight();
        if (flight.getId() == id) {
            flight.clear(true, FlightData::CreationTimeMode::Reset);
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

bool FlightService::updateTitle(Flight &flight, const QString &title) const noexcept
{
    bool ok {false};
    const std::int64_t flightId = flight.getId();
    if (flightId != Const::InvalidId) {
        if (flightId != Const::RecordingId) {
            ok = updateTitle(flightId, title);
        } else {
            ok = true;
        }
        if (ok) {
            flight.setTitle(title);
        }
    }
    return ok;
}

bool FlightService::updateFlightNumber(Flight &flight, const QString &flightNumber) const noexcept
{
    bool ok {false};
    const std::int64_t flightId = flight.getId();
    if (flightId != Const::InvalidId) {
        if (flightId != Const::RecordingId) {
            ok = updateFlightNumber(flightId, flightNumber);
        } else {
            ok = true;
        }
        if (ok) {
            flight.setFlightNumber(flightNumber);
        }
    }
    return ok;
}

bool FlightService::updateTitle(std::int64_t id, const QString &title) const noexcept
{
    bool ok {false};
    if (Flight::isValidId(id)) {
        QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        ok = db.transaction();
        if (ok) {
            ok = d->flightDao->updateTitle(id, title);
            if (ok) {
                ok = db.commit();
                emit Logbook::getInstance().getCurrentFlight().titleChanged(id, title);
            } else {
                db.rollback();
            }
#ifdef DEBUG
        } else {
            qDebug() << "FlightService::updateTitle: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
        }
    }
    return ok;
}

bool FlightService::updateFlightNumber(std::int64_t id, const QString &flightNumber) const noexcept
{
    bool ok {false};
    if (Flight::isValidId(id)) {
        QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        ok = db.transaction();
        if (ok) {
            ok = d->flightDao->updateFlightNumber(id, flightNumber);
            if (ok) {
                ok = db.commit();
                emit Logbook::getInstance().getCurrentFlight().flightNumberChanged(id, flightNumber);
            } else {
                db.rollback();
            }
#ifdef DEBUG
        } else {
            qDebug() << "FlightService::updateFlightNumber: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
        }
    }
    return ok;
}

bool FlightService::updateDescription(Flight &flight, const QString &description) const noexcept
{
    bool ok {false};
    const std::int64_t flightId = flight.getId();
    if (flightId != Const::InvalidId) {
        if (flightId != Const::RecordingId) {
            ok = updateDescription(flight.getId(), description);
        } else {
            ok = true;
        }
        if (ok) {
            flight.setDescription(description);
        }
    }
    return ok;
}

bool FlightService::updateDescription(std::int64_t id, const QString &description) const noexcept
{
    bool ok {false};
    if (Flight::isValidId(id)) {
        QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        ok = db.transaction();
        if (ok) {
            ok = d->flightDao->updateDescription(id, description);
            if (ok) {
                ok = db.commit();
                emit Logbook::getInstance().getCurrentFlight().descriptionChanged(id, description);
            } else {
                db.rollback();
            }
#ifdef DEBUG
        } else {
            qDebug() << "FlightService::updateDescription: SQL error:" << db.lastError().text() << "- error code:" << db.lastError().nativeErrorCode();
#endif
        }
    }
    return ok;
}

bool FlightService::updateUserAircraftIndex(Flight &flight, int index) const noexcept
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
