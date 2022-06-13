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

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QTimeZone>

#include <Kernel/Enum.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/PrimaryFlightControl.h>
#include <Model/PrimaryFlightControlData.h>
#include <Model/SecondaryFlightControl.h>
#include <Model/SecondaryFlightControlData.h>
#include <Model/AircraftHandle.h>
#include <Model/AircraftHandleData.h>
#include <Model/Light.h>
#include <Model/LightData.h>
#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include "../../Dao/FlightDaoIntf.h"
#include "../../Dao/AircraftTypeDaoIntf.h"
#include "../../Dao/AircraftDaoIntf.h"
#include "../../Dao/PositionDaoIntf.h"
#include "../../Dao/EngineDaoIntf.h"
#include "../../Dao/PrimaryFlightControlDaoIntf.h"
#include "../../Dao/SecondaryFlightControlDaoIntf.h"
#include "../../Dao/HandleDaoIntf.h"
#include "../../Dao/LightDaoIntf.h"
#include "../../Dao/WaypointDaoIntf.h"
#include "../../Dao/DaoFactory.h"
#include "SQLiteAircraftDao.h"

class SQLiteAircraftDaoPrivate
{
public:
    SQLiteAircraftDaoPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          aircraftTypeDao(daoFactory->createAircraftTypeDao()),
          positionDao(daoFactory->createPositionDao()),
          engineDao(daoFactory->createEngineDao()),
          primaryFlightControlDao(daoFactory->createPrimaryFlightControlDao()),
          secondaryFlightControlDao(daoFactory->createSecondaryFlightControlDao()),
          handleDao(daoFactory->createHandleDao()),
          lightDao(daoFactory->createLightDao()),
          waypointDao(daoFactory->createFlightPlanDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftTypeDaoIntf> aircraftTypeDao;
    std::unique_ptr<PositionDaoIntf> positionDao;
    std::unique_ptr<EngineDaoIntf> engineDao;
    std::unique_ptr<PrimaryFlightControlDaoIntf> primaryFlightControlDao;
    std::unique_ptr<SecondaryFlightControlDaoIntf> secondaryFlightControlDao;
    std::unique_ptr<HandleDaoIntf> handleDao;
    std::unique_ptr<LightDaoIntf> lightDao;
    std::unique_ptr<WaypointDaoIntf> waypointDao;
};

// PUBLIC

SQLiteAircraftDao::SQLiteAircraftDao() noexcept
    : d(std::make_unique<SQLiteAircraftDaoPrivate>())
{}

SQLiteAircraftDao::~SQLiteAircraftDao() noexcept
{}

bool SQLiteAircraftDao::add(std::int64_t flightId, std::size_t sequenceNumber, Aircraft &aircraft)  noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into aircraft ("
        "  flight_id,"
        "  seq_nr,"
        "  type,"
        "  time_offset,"
        "  tail_number,"
        "  airline,"
        "  flight_number,"
        "  initial_airspeed,"
        "  altitude_above_ground,"
        "  start_on_ground"
        ") values ("
        " :flight_id,"
        " :seq_nr,"
        " :type,"
        " :time_offset,"
        " :tail_number,"
        " :airline,"
        " :flight_number,"
        " :initial_airspeed,"
        " :altitude_above_ground,"
        " :start_on_ground"
        ");");

    const AircraftType &aircraftType = aircraft.getAircraftInfo().aircraftType;
    bool ok = d->aircraftTypeDao->upsert(aircraftType);
    if (ok) {
        const AircraftInfo &info = aircraft.getAircraftInfo();
        query.bindValue(":flight_id", QVariant::fromValue(flightId));
        query.bindValue(":seq_nr", sequenceNumber);
        query.bindValue(":type", aircraftType.type);
        query.bindValue(":time_offset", QVariant::fromValue(info.timeOffset));
        query.bindValue(":tail_number", info.tailNumber);
        query.bindValue(":airline", info.airline);
        query.bindValue(":flight_number", info.flightNumber);
        query.bindValue(":initial_airspeed", info.initialAirspeed);
        query.bindValue(":altitude_above_ground", info.altitudeAboveGround);
        query.bindValue(":start_on_ground", info.startOnGround);

        ok = query.exec();
    }
    if (ok) {
        std::int64_t id = query.lastInsertId().toLongLong(&ok);
        aircraft.setId(id);
#ifdef DEBUG
    } else {
        qDebug("SQLiteAircraftDao::add: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }
    if (ok) {
        for (const PositionData &data : aircraft.getPosition()) {
            ok = d->positionDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const EngineData &data : aircraft.getEngine()) {
            ok = d->engineDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const PrimaryFlightControlData &data : aircraft.getPrimaryFlightControl()) {
            ok = d->primaryFlightControlDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const SecondaryFlightControlData &data : aircraft.getSecondaryFlightControl()) {
            ok = d->secondaryFlightControlDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const AircraftHandleData &data : aircraft.getAircraftHandle()) {
            ok = d->handleDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const LightData &data : aircraft.getLight()) {
            ok = d->lightDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        ok = d->waypointDao->add(aircraft.getId(), aircraft.getFlightPlan());
    }
    return ok;
}

bool SQLiteAircraftDao::getByFlightId(std::int64_t flightId, std::back_insert_iterator<std::vector<std::unique_ptr<Aircraft>>> backInsertIterator) const noexcept
{
    std::vector<AircraftInfo> aircraftInfos;
    bool ok = getAircraftInfosByFlightId(flightId, aircraftInfos);
    if (ok) {
        for (const AircraftInfo &info: aircraftInfos) {
            std::unique_ptr<Aircraft> aircraft = std::make_unique<Aircraft>();
            aircraft->setId(info.aircraftId);
            aircraft->setAircraftInfo(info);
            ok = d->positionDao->getByAircraftId(aircraft->getId(), aircraft->getPosition().backInsertIterator());
            if (ok) {
                ok = d->engineDao->getByAircraftId(aircraft->getId(), aircraft->getEngine().backInsertIterator());
            }
            if (ok) {
                ok = d->primaryFlightControlDao->getByAircraftId(aircraft->getId(), aircraft->getPrimaryFlightControl().backInsertIterator());
            }
            if (ok) {
                ok = d->secondaryFlightControlDao->getByAircraftId(aircraft->getId(), aircraft->getSecondaryFlightControl().backInsertIterator());
            }
            if (ok) {
                ok = d->handleDao->getByAircraftId(aircraft->getId(), aircraft->getAircraftHandle().backInsertIterator());
            }
            if (ok) {
                ok = d->lightDao->getByAircraftId(aircraft->getId(), aircraft->getLight().backInsertIterator());
            }
            if (ok) {
                ok = d->waypointDao->getByAircraftId(aircraft->getId(), aircraft->getFlightPlan());
            }
            if (ok) {
                backInsertIterator = std::move(aircraft);
            }
        }
    }

    return ok;
}

bool SQLiteAircraftDao::adjustAircraftSequenceNumbersByFlightId(std::int64_t flightId, std::size_t sequenceNumber) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update aircraft "
        "set    seq_nr = seq_nr - 1 "
        "where flight_id = :flight_id "
        "and   seq_nr    > :seq_nr;"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    query.bindValue(":seq_nr", sequenceNumber);
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteAircraftDao::adjustAircraftSequenceNumbersByIndex: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteAircraftDao::deleteAllByFlightId(std::int64_t flightId) noexcept
{
    // Delete "bottom-up" in order not to violate foreign key constraints
    bool ok = d->positionDao->deleteByFlightId(flightId);
    if (ok) {
        ok = d->engineDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->primaryFlightControlDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->secondaryFlightControlDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->handleDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->lightDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->waypointDao->deleteByFlightId(flightId);
    }
    if (ok) {
        QSqlQuery query;
        query.prepare(
            "delete "
            "from   aircraft "
            "where  flight_id = :flight_id;"
        );
        query.bindValue(":flight_id", QVariant::fromValue(flightId));
        ok = query.exec();
#ifdef DEBUG
        if (!ok) {
            qDebug("SQLiteAircraftDao::deleteByFlightId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
        }
#endif
    }

    return ok;
}

bool SQLiteAircraftDao::deleteById(std::int64_t id) noexcept
{
    // Delete "bottom-up" in order not to violate foreign key constraints
    // Note: aircraft types (table aircraft_type) are not deleted
    bool ok = d->positionDao->deleteByAircraftId(id);
    if (ok) {
        ok = d->engineDao->deleteByAircraftId(id);
    }
    if (ok) {
        ok = d->primaryFlightControlDao->deleteByAircraftId(id);
    }
    if (ok) {
        ok = d->secondaryFlightControlDao->deleteByAircraftId(id);
    }
    if (ok) {
        ok = d->handleDao->deleteByAircraftId(id);
    }
    if (ok) {
        ok = d->lightDao->deleteByAircraftId(id);
    }
    if (ok) {
        ok = d->waypointDao->deleteByAircraftId(id);
    }
    if (ok) {
        QSqlQuery query;
        query.prepare(
            "delete "
            "from   aircraft "
            "where  id = :id;"
        );
        query.bindValue(":id", QVariant::fromValue(id));
        ok = query.exec();
#ifdef DEBUG
        if (!ok) {
            qDebug("SQLiteAircraftDao::deleteById: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
        }
#endif
    }

    return ok;
}

bool SQLiteAircraftDao::getAircraftInfosByFlightId(std::int64_t flightId, std::vector<AircraftInfo> &aircraftInfos) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   aircraft a "
        "where  a.flight_id = :flight_id "
        "order by a.seq_nr;"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    bool ok = query.exec();
    if (ok) {
        aircraftInfos.clear();
        QSqlRecord record = query.record();
        const int idIdx = record.indexOf("id");
        const int typeIdx = record.indexOf("type");
        const int timeOffsetIdx = record.indexOf("time_offset");
        const int tailNumberIdx = record.indexOf("tail_number");
        const int airlineIdx = record.indexOf("airline");
        const int flightNumberIdx = record.indexOf("flight_number");
        const int initialAirspeedIdx = record.indexOf("initial_airspeed");
        const int airCraftAltitudeAboveGroundIdx = record.indexOf("altitude_above_ground");
        const int startOnGroundIdx = record.indexOf("start_on_ground");
        while (ok && query.next()) {
            AircraftInfo info(query.value(idIdx).toLongLong());
            const QString &type = query.value(typeIdx).toString();
            info.timeOffset = query.value(timeOffsetIdx).toULongLong();
            info.tailNumber = query.value(tailNumberIdx).toString();
            info.airline = query.value(airlineIdx).toString();
            info.flightNumber = query.value(flightNumberIdx).toString();
            info.initialAirspeed = query.value(initialAirspeedIdx).toInt();
            info.altitudeAboveGround = query.value(airCraftAltitudeAboveGroundIdx).toInt();
            info.startOnGround = query.value(startOnGroundIdx).toBool();

            AircraftType aircraftType;
            ok = d->aircraftTypeDao->getByType(type, aircraftType);
            if (ok) {
                info.aircraftType = std::move(aircraftType);
                aircraftInfos.push_back(info);
            };
        }
    }
#ifdef DEBUG
    else {
        qDebug("SQLiteAircraftDao::getAircraftInfosByFlightId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteAircraftDao::updateTimeOffset(std::int64_t id, std::int64_t timeOffset) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update aircraft "
        "set    time_offset = :time_offset "
        "where  id = :id;"
    );

    query.bindValue(":time_offset", QVariant::fromValue(timeOffset));
    query.bindValue(":id", QVariant::fromValue(id));
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteAircraftDao::updateTimeOffset: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
};

bool SQLiteAircraftDao::updateTailNumber(std::int64_t id, const QString &tailNumber) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update aircraft "
        "set    tail_number = :tail_number "
        "where  id = :id;"
    );

    query.bindValue(":tail_number", tailNumber);
    query.bindValue(":id", QVariant::fromValue(id));
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteAircraftDao::updateTailNumber: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
};
