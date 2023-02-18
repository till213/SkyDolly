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
#include <vector>
#include <cstdint>

#include <QSqlDatabase>

#include <Kernel/Const.h>
#include <Model/Aircraft.h>
#include <Model/Logbook.h>
#include "../Dao/FlightDaoIntf.h"
#include "../Dao/DaoFactory.h"
#include "../Dao/AircraftDaoIntf.h"
#include <Service/AircraftService.h>

struct AircraftServicePrivate
{
public:
    AircraftServicePrivate(QString connectionName) noexcept
        : connectionName(connectionName),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite, std::move(connectionName))),
          aircraftDao(daoFactory->createAircraftDao()),
          flightDao(daoFactory->createFlightDao())
    {}

    QString connectionName;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftDaoIntf> aircraftDao;
    std::unique_ptr<FlightDaoIntf> flightDao;
};

// PUBLIC

AircraftService::AircraftService(QString connectionName) noexcept
    : d(std::make_unique<AircraftServicePrivate>(std::move(connectionName)))
{}

AircraftService::AircraftService(AircraftService &&rhs) noexcept = default;
AircraftService &AircraftService::operator=(AircraftService &&rhs) noexcept = default;
AircraftService::~AircraftService() = default;

bool AircraftService::store(std::int64_t flightId, std::size_t sequenceNumber, Aircraft &aircraft) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        ok = d->aircraftDao->add(flightId, sequenceNumber, aircraft);
        if (ok) {
            ok = d->flightDao->updateUserAircraftIndex(flight.getId(), flight.getUserAircraftIndex());
        }
        if (ok) {
            ok = db.commit();
            if (ok) {
                emit flight.aircraftStored();
            }
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool AircraftService::exportAircraft(std::int64_t flightId, std::size_t sequenceNumber, const Aircraft &aircraft) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        ok = d->aircraftDao->exportAircraft(flightId, sequenceNumber, aircraft);
        if (ok) {
            ok = d->flightDao->updateUserAircraftIndex(flight.getId(), flight.getUserAircraftIndex());
        }
        if (ok) {
            ok = db.commit();
            if (ok) {
                emit flight.aircraftStored();
            }
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool AircraftService::deleteByIndex(int index) noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    const std::int64_t aircraftId = flight.removeAircraftByIndex(index);
    bool ok {true};
    if (aircraftId != Const::InvalidId) {
        QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        ok = db.transaction();
        if (ok) {
            ok = d->aircraftDao->deleteById(aircraftId);
            if (ok) {
                ok = d->flightDao->updateUserAircraftIndex(flight.getId(), flight.getUserAircraftIndex());
            }
            if (ok) {
                // Sequence numbers start at 1
                ok = d->aircraftDao->adjustAircraftSequenceNumbersByFlightId(flight.getId(), static_cast<std::int64_t>(index) + 1);
            }
            if (ok) {
                ok = db.commit();
            } else {
                db.rollback();
            }
        }
    }
    return ok;
}

std::vector<AircraftInfo> AircraftService::getAircraftInfos(std::int64_t flightId, bool *ok) const noexcept
{
    std::vector<AircraftInfo> aircraftInfos;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool success = db.transaction();
    if (success) {
        aircraftInfos = d->aircraftDao->getAircraftInfosByFlightId(flightId, &success);
        db.rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftInfos;
}

bool AircraftService::changeTimeOffset(Aircraft &aircraft, std::int64_t newOffset) noexcept
{
    bool ok {false};
    const std::int64_t aircraftId = aircraft.getId();
    if (aircraftId != Const::InvalidId) {
        if (aircraftId != Const::RecordingId) {
            QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
            ok = db.transaction();
            if (ok) {
                ok = d->aircraftDao->updateTimeOffset(aircraft.getId(), newOffset);
                if (ok) {
                    ok = db.commit();
                } else {
                    db.rollback();
                }
            }
        } else {
            ok = true;
        }
        if (ok) {
            aircraft.setTimeOffset(newOffset);
            emit Logbook::getInstance().getCurrentFlight().timeOffsetChanged(aircraft);
        }
    }
    return ok;
}

bool AircraftService::changeTailNumber(Aircraft &aircraft, const QString &tailNumber) noexcept
{
    bool ok {false};
    const std::int64_t aircraftId = aircraft.getId();
    if (aircraftId != Const::InvalidId) {
        if (aircraftId != Const::RecordingId) {
            QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
            ok = db.transaction();
            if (ok) {
                ok = d->aircraftDao->updateTailNumber(aircraft.getId(), tailNumber);
                if (ok) {
                    ok = db.commit();
                } else {
                    db.rollback();
                }
            }
        } else {
            ok = true;
        }
        if (ok) {
            aircraft.setTailNumber(tailNumber);
            emit Logbook::getInstance().getCurrentFlight().tailNumberChanged(aircraft);
        }
    }
    return ok;
}
