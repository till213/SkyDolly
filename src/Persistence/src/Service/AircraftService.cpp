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
#include <cstdint>

#include <QSqlDatabase>

#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/Logbook.h"
#include "../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../Dao/FlightDaoIntf.h"
#include "../Dao/DaoFactory.h"
#include "../Dao/AircraftDaoIntf.h"
#include "AircraftService.h"

class AircraftServicePrivate
{
public:
    AircraftServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          aircraftDao(daoFactory->createAircraftDao()),
          flightDao(daoFactory->createFlightDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftDaoIntf> aircraftDao;
    std::unique_ptr<FlightDaoIntf> flightDao;
};

// PUBLIC

AircraftService::AircraftService() noexcept
    : d(std::make_unique<AircraftServicePrivate>())
{}

AircraftService::~AircraftService() noexcept
{}

bool AircraftService::store(std::int64_t flightId, int sequenceNumber, Aircraft &aircraft) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        ok = d->aircraftDao->add(flightId, sequenceNumber, aircraft);
        if (ok) {
            ok = d->flightDao->updateUserAircraftIndex(flight.getId(), flight.getUserAircraftIndex());
        }
        if (ok) {
            ok = QSqlDatabase::database().commit();
            emit flight.aircraftStored(aircraft);
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool AircraftService::deleteByIndex(int index) noexcept
{
    // Remove AI object
    Flight &flight = Logbook::getInstance().getCurrentFlight();

    const bool removeUserAircraft = flight.getUserAircraftIndex() == index;
    Aircraft &aircraft = flight[index];
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().destroyAIObject(aircraft);
    }
    const std::int64_t aircraftId = flight.deleteAircraftByIndex(index);
    bool ok;
    if (aircraftId != Aircraft::InvalidId) {
        if (skyConnect && removeUserAircraft) {
            skyConnect->get().updateUserAircraft();
        }
        ok = QSqlDatabase::database().transaction();
        if (ok) {
            ok = d->aircraftDao->deleteById(aircraftId);
            if (ok) {
                ok = d->flightDao->updateUserAircraftIndex(flight.getId(), flight.getUserAircraftIndex());
            }
            if (ok) {
                // Sequence numbers start at 1
                ok = d->aircraftDao->adjustAircraftSequenceNumbersByFlightId(flight.getId(), index + 1);
            }
            if (ok) {
                ok = QSqlDatabase::database().commit();
            } else {
                QSqlDatabase::database().rollback();
            }
        }
    } else {
        ok = true;
    }
    return ok;
}

bool AircraftService::getAircraftInfos(std::int64_t flightId, std::vector<AircraftInfo> &aircraftInfos) const noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->aircraftDao->getAircraftInfosByFlightId(flightId, aircraftInfos);
        QSqlDatabase::database().rollback();
    }
    return ok;
}

bool AircraftService::changeTimeOffset(Aircraft &aircraft, std::int64_t newOffset) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->aircraftDao->updateTimeOffset(aircraft.getId(), newOffset);
        if (ok) {
            aircraft.setTimeOffset(newOffset);
            std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
            if (skyConnect && !skyConnect->get().isReplaying()) {
                // Update the aircraft position in the flight simulator
                skyConnect->get().seek(skyConnect->get().getCurrentTimestamp());
            }
            ok = QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool AircraftService::changeTailNumber(Aircraft &aircraft, const QString &tailNumber) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->aircraftDao->updateTailNumber(aircraft.getId(), tailNumber);
        if (ok) {
            aircraft.setTailNumber(tailNumber);
            std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
            if (skyConnect) {
                skyConnect->get().updateAIObjects();
            }
            ok = QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}
