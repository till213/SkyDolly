/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <QObject>
#include <QSqlDatabase>

#include <memory>
#include <utility>
#include <forward_list>

#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../SkyConnect/src/SkyManager.h"
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
    SkyConnectIntf &skyConnect = SkyManager::getInstance().getCurrentSkyConnect();
    if (skyConnect.isConnected()) {
        skyConnect.destroyAIObjects();
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
            QSqlDatabase::database().commit();
             emit flightStored(flight.getId());
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool FlightService::restore(qint64 id, Flight &flight) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        SkyConnectIntf &skyConnect = SkyManager::getInstance().getCurrentSkyConnect();
        skyConnect.destroyAIObjects();
        ok = d->flightDao->getFlightById(id, flight);
        QSqlDatabase::database().rollback();
        emit flightRestored(flight.getId());
        if (ok) {
            ok = skyConnect.createAIObjects();
        }
    }
    return ok;
}

bool FlightService::deleteById(qint64 id)  noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->flightDao->deleteById(id);
        if (ok) {
            QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool FlightService::updateTitle(qint64 id, const QString &title) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->flightDao->updateTitle(id, title);
        if (ok) {
            QSqlDatabase::database().commit();
            emit flightUpdated(id);
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool FlightService::updateTitleAndDescription(qint64 id, const QString &title, const QString &description) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->flightDao->updateTitleAndDescription(id, title, description);
        if (ok) {
            QSqlDatabase::database().commit();
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
        SkyConnectIntf &skyConnect = SkyManager::getInstance().getCurrentSkyConnect();
        skyConnect.updateUserAircraft();
        ok = d->flightDao->updateUserAircraftIndex(flight.getId(), index);
        if (ok) {
            QSqlDatabase::database().commit();
            emit flightUpdated(flight.getId());
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

std::forward_list<FlightDate> FlightService::getFlightDates() const noexcept
{
    std::forward_list<FlightDate> flightDates;
    if (QSqlDatabase::database().transaction()) {
        // TODO IMPLEMENT ME!!!
        flightDates.emplace_front(2010, 1, 5);
        flightDates.emplace_front(2010, 1, 6);
        flightDates.emplace_front(2010, 5, 6);
        flightDates.emplace_front(2010, 6, 6);
        flightDates.emplace_front(2015, 1, 6);
        flightDates.emplace_front(2016, 5, 20);
        flightDates.emplace_front(2021, 4, 1);
        flightDates.emplace_front(2021, 4, 2);
        flightDates.emplace_front(2021, 4, 3);
        flightDates.emplace_front(2021, 5, 3);
        QSqlDatabase::database().rollback();
    }
    return flightDates;
}

QVector<FlightSummary> FlightService::getFlightSummaries() const noexcept
{
    QVector<FlightSummary> descriptions;
    if (QSqlDatabase::database().transaction()) {
        descriptions = d->flightDao->getFlightSummaries();
        QSqlDatabase::database().rollback();
    }
    return descriptions;
}
