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
#include <forward_list>
#include <cstdint>

#include <QObject>
#include <QSqlDatabase>

#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../Dao/DaoFactory.h"
#include "../Dao/FlightDaoIntf.h"
#include "../Dao/AircraftDaoIntf.h"
#include "FlightService.h"

class FlightServicePrivate
{
public:
    FlightServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          flightDao(daoFactory->createFlightDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<FlightDaoIntf> flightDao;
};

// PUBLIC

FlightService::FlightService(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<FlightServicePrivate>())
{}

FlightService::~FlightService() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect && skyConnect->get().isConnected()) {
        skyConnect->get().destroyAIObjects();
    }
#ifdef DEBUG
    qDebug("FlightService::~FlightService: DELETED.");
#endif
}

bool FlightService::store(Flight &flight) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->flightDao->addFlight(flight);
        if (ok) {
            ok = QSqlDatabase::database().commit();
            // TODO Refactor: remove all signals from services, move them to Flight
            emit flightStored(flight.getId());
            emit flight.flightStored(flight.getId());
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool FlightService::restore(std::int64_t id, Flight &flight) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
        if (skyConnect) {
            skyConnect->get().destroyAIObjects();
            ok = d->flightDao->getFlightById(id, flight);
            emit flightRestored(flight.getId());
            if (ok) {
                ok = skyConnect->get().createAIObjects();
            }
        }
        QSqlDatabase::database().rollback();
    }
    return ok;
}

bool FlightService::deleteById(std::int64_t id)  noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->flightDao->deleteById(id);
        if (ok) {
            ok = QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
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
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->flightDao->updateTitle(id, title);
        if (ok) {
            ok = QSqlDatabase::database().commit();
            emit flightUpdated(id);
        } else {
            QSqlDatabase::database().rollback();
        }
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
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->flightDao->updateTitleAndDescription(id, title, description);
        if (ok) {
            ok = QSqlDatabase::database().commit();
            emit flightUpdated(id);
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool FlightService::updateUserAircraftIndex(Flight &flight, int index) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        flight.setUserAircraftIndex(index);
        std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
        if (skyConnect) {
            skyConnect->get().updateUserAircraft();
        }
        ok = d->flightDao->updateUserAircraftIndex(flight.getId(), index);
        if (ok) {
            ok = QSqlDatabase::database().commit();
            emit flightUpdated(flight.getId());
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}
