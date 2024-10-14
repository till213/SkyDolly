/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <utility>

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QTimeZone>
#include <QSqlDriver>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Attitude.h>
#include <Model/AttitudeData.h>
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
#include "../../Dao/AircraftTypeDaoIntf.h"
#include "../../Dao/PositionDaoIntf.h"
#include "../../Dao/AttitudeDaoIntf.h"
#include "../../Dao/EngineDaoIntf.h"
#include "../../Dao/PrimaryFlightControlDaoIntf.h"
#include "../../Dao/SecondaryFlightControlDaoIntf.h"
#include "../../Dao/HandleDaoIntf.h"
#include "../../Dao/LightDaoIntf.h"
#include "../../Dao/WaypointDaoIntf.h"
#include "../../Dao/DaoFactory.h"
#include "SQLiteAircraftDao.h"

struct SQLiteAircraftDaoPrivate
{
    SQLiteAircraftDaoPrivate(QString connectionName) noexcept
        : connectionName(connectionName),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite, std::move(connectionName))),
          aircraftTypeDao(daoFactory->createAircraftTypeDao()),
          positionDao(daoFactory->createPositionDao()),
          attitudeDao(daoFactory->createAttitudeDao()),
          engineDao(daoFactory->createEngineDao()),
          primaryFlightControlDao(daoFactory->createPrimaryFlightControlDao()),
          secondaryFlightControlDao(daoFactory->createSecondaryFlightControlDao()),
          handleDao(daoFactory->createHandleDao()),
          lightDao(daoFactory->createLightDao()),
          waypointDao(daoFactory->createFlightPlanDao())
    {}

    QString connectionName;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftTypeDaoIntf> aircraftTypeDao;
    std::unique_ptr<PositionDaoIntf> positionDao;
    std::unique_ptr<AttitudeDaoIntf> attitudeDao;
    std::unique_ptr<EngineDaoIntf> engineDao;
    std::unique_ptr<PrimaryFlightControlDaoIntf> primaryFlightControlDao;
    std::unique_ptr<SecondaryFlightControlDaoIntf> secondaryFlightControlDao;
    std::unique_ptr<HandleDaoIntf> handleDao;
    std::unique_ptr<LightDaoIntf> lightDao;
    std::unique_ptr<WaypointDaoIntf> waypointDao;
};

namespace
{
    // The initial capacity of the position vector (e.g. SQLite does not support returning
    // the result count for the given SELECT query)
    // Most flights have only one aircraft
    constexpr int DefaultCapacity = 1;
}

// PUBLIC

SQLiteAircraftDao::SQLiteAircraftDao(QString connectionName) noexcept
    : d {std::make_unique<SQLiteAircraftDaoPrivate>(std::move(connectionName))}
{}

SQLiteAircraftDao::SQLiteAircraftDao(SQLiteAircraftDao &&rhs) noexcept = default;
SQLiteAircraftDao &SQLiteAircraftDao::operator=(SQLiteAircraftDao &&rhs) noexcept = default;
SQLiteAircraftDao::~SQLiteAircraftDao() = default;

bool SQLiteAircraftDao::add(std::int64_t flightId, std::size_t sequenceNumber, Aircraft &aircraft) const noexcept
{
    bool ok {false};
    const std::int64_t aircraftId = insertAircraft(flightId, sequenceNumber, aircraft);
    if (aircraftId != Const::InvalidId) {
        aircraft.setId(aircraftId);
        ok = insertAircraftData(aircraftId, aircraft);
    }
    return ok;
}

bool SQLiteAircraftDao::exportAircraft(std::int64_t flightId, std::size_t sequenceNumber, const Aircraft &aircraft) const noexcept
{
    bool ok {false};
    const std::int64_t aircraftId = insertAircraft(flightId, sequenceNumber, aircraft);
    if (aircraftId != Const::InvalidId) {
        ok = insertAircraftData(aircraftId, aircraft);
    }
    return ok;
}

std::vector<Aircraft> SQLiteAircraftDao::getByFlightId(std::int64_t flightId, bool *ok) const noexcept
{
    std::vector<Aircraft> aircraftList;
    bool success {true};
    std::vector<AircraftInfo> aircraftInfos = getAircraftInfosByFlightId(flightId, &success);
    if (success) {
        aircraftList.reserve(aircraftInfos.size());
        for (const auto &info: aircraftInfos) {
            Aircraft aircraft;
            aircraft.setId(info.aircraftId);
            aircraft.setAircraftInfo(info);
            aircraft.getPosition().setData(d->positionDao->getByAircraftId(aircraft.getId(), &success));
            if (success) {
                aircraft.getAttitude().setData(d->attitudeDao->getByAircraftId(aircraft.getId(), &success));
            }
            if (success) {
                aircraft.getEngine().setData(d->engineDao->getByAircraftId(aircraft.getId(), &success));
            }
            if (success) {
                aircraft.getPrimaryFlightControl().setData(d->primaryFlightControlDao->getByAircraftId(aircraft.getId(), &success));
            }
            if (success) {
                aircraft.getSecondaryFlightControl().setData(d->secondaryFlightControlDao->getByAircraftId(aircraft.getId(), &success));
            }
            if (success) {
                aircraft.getAircraftHandle().setData(d->handleDao->getByAircraftId(aircraft.getId(), &success));
            }
            if (success) {
                aircraft.getLight().setData(d->lightDao->getByAircraftId(aircraft.getId(), &success));
            }
            if (success) {
                success = d->waypointDao->getByAircraftId(aircraft.getId(), aircraft.getFlightPlan());
            }
            if (success) {
                aircraftList.push_back(std::move(aircraft));
            }
        }
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftList;
}

bool SQLiteAircraftDao::adjustAircraftSequenceNumbersByFlightId(std::int64_t flightId, std::size_t sequenceNumber) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update aircraft "
        "set    seq_nr = seq_nr - 1 "
        "where flight_id = :flight_id "
        "and   seq_nr    > :seq_nr;"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    query.bindValue(":seq_nr", QVariant::fromValue(sequenceNumber));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteAircraftDao::adjustAircraftSequenceNumbersByIndex: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteAircraftDao::deleteAllByFlightId(std::int64_t flightId) const noexcept
{
    // Delete "bottom-up" in order not to violate foreign key constraints
    bool ok = d->positionDao->deleteByFlightId(flightId);
    if (ok) {
        ok = d->attitudeDao->deleteByFlightId(flightId);
    }
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
        const auto db {QSqlDatabase::database(d->connectionName)};
        QSqlQuery query {db};
        query.prepare(
            "delete "
            "from   aircraft "
            "where  flight_id = :flight_id;"
        );
        query.bindValue(":flight_id", QVariant::fromValue(flightId));
        ok = query.exec();
#ifdef DEBUG
        if (!ok) {
            qDebug() << "SQLiteAircraftDao::deleteByFlightId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
        }
#endif
    }

    return ok;
}

bool SQLiteAircraftDao::deleteById(std::int64_t id) const noexcept
{
    // Delete "bottom-up" in order not to violate foreign key constraints
    // Note: aircraft types (table aircraft_type) are not deleted
    bool ok = d->positionDao->deleteByAircraftId(id);
    if (ok) {
        ok = d->attitudeDao->deleteByAircraftId(id);
    }
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
        const auto db {QSqlDatabase::database(d->connectionName)};
        QSqlQuery query {db};
        query.prepare(
            "delete "
            "from   aircraft "
            "where  id = :id;"
        );
        query.bindValue(":id", QVariant::fromValue(id));
        ok = query.exec();
#ifdef DEBUG
        if (!ok) {
            qDebug() << "SQLiteAircraftDao::deleteById: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
        }
#endif
    }

    return ok;
}

std::vector<AircraftInfo> SQLiteAircraftDao::getAircraftInfosByFlightId(std::int64_t flightId,  bool *ok) const noexcept
{
    std::vector<AircraftInfo> aircraftInfos;
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   aircraft a "
        "where  a.flight_id = :flight_id "
        "order by a.seq_nr;"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    bool success = query.exec();
    if (success) {
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            aircraftInfos.reserve(query.size());
        } else {
            aircraftInfos.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const auto idIdx = record.indexOf("id");
        const auto typeIdx = record.indexOf("type");
        const auto timeOffsetIdx = record.indexOf("time_offset");
        const auto tailNumberIdx = record.indexOf("tail_number");
        const auto airlineIdx = record.indexOf("airline");
        const auto initialAirspeedIdx = record.indexOf("initial_airspeed");
        const auto airCraftAltitudeAboveGroundIdx = record.indexOf("altitude_above_ground");
        const auto startOnGroundIdx = record.indexOf("start_on_ground");
        while (success && query.next()) {
            AircraftInfo info(query.value(idIdx).toLongLong());
            const QString &type = query.value(typeIdx).toString();
            info.timeOffset = query.value(timeOffsetIdx).toLongLong();
            info.tailNumber = query.value(tailNumberIdx).toString();
            info.airline = query.value(airlineIdx).toString();
            info.initialAirspeed = query.value(initialAirspeedIdx).toInt();
            info.altitudeAboveGround = query.value(airCraftAltitudeAboveGroundIdx).toFloat();
            info.startOnGround = query.value(startOnGroundIdx).toBool();

            AircraftType aircraftType = d->aircraftTypeDao->getByType(type, &success);
            if (success) {
                info.aircraftType = std::move(aircraftType);
                aircraftInfos.push_back(std::move(info));
            };
        }
    }
#ifdef DEBUG
    else {
        qDebug() << "SQLiteAircraftDao::getAircraftInfosByFlightId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftInfos;
}

bool SQLiteAircraftDao::updateTimeOffset(std::int64_t id, std::int64_t timeOffset) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update aircraft "
        "set    time_offset = :time_offset "
        "where  id = :id;"
    );

    query.bindValue(":time_offset", QVariant::fromValue(timeOffset));
    query.bindValue(":id", QVariant::fromValue(id));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteAircraftDao::updateTimeOffset: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
};

bool SQLiteAircraftDao::updateTailNumber(std::int64_t id, const QString &tailNumber) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update aircraft "
        "set    tail_number = :tail_number "
        "where  id = :id;"
    );

    query.bindValue(":tail_number", tailNumber);
    query.bindValue(":id", QVariant::fromValue(id));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteAircraftDao::updateTailNumber: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
};

// PRIVATE

inline std::int64_t SQLiteAircraftDao::insertAircraft(std::int64_t flightId, std::size_t sequenceNumber, const Aircraft &aircraft) const noexcept
{
    std::int64_t aircraftId {Const::InvalidId};
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "insert into aircraft ("
        "  flight_id,"
        "  seq_nr,"
        "  type,"
        "  time_offset,"
        "  tail_number,"
        "  airline,"
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
        " :initial_airspeed,"
        " :altitude_above_ground,"
        " :start_on_ground"
        ");"
    );

    const AircraftType &aircraftType = aircraft.getAircraftInfo().aircraftType;
    bool ok = d->aircraftTypeDao->upsert(aircraftType);
    if (ok) {
        const auto &info = aircraft.getAircraftInfo();
        query.bindValue(":flight_id", QVariant::fromValue(flightId));
        query.bindValue(":seq_nr", QVariant::fromValue(sequenceNumber));
        query.bindValue(":type", aircraftType.type);
        query.bindValue(":time_offset", QVariant::fromValue(info.timeOffset));
        query.bindValue(":tail_number", info.tailNumber);
        query.bindValue(":airline", info.airline);
        query.bindValue(":initial_airspeed", info.initialAirspeed);
        query.bindValue(":altitude_above_ground", info.altitudeAboveGround);
        query.bindValue(":start_on_ground", info.startOnGround);

        ok = query.exec();
    }
    if (ok) {
        aircraftId = query.lastInsertId().toLongLong(&ok);
    } else {
        aircraftId = Const::InvalidId;
#ifdef DEBUG
        qDebug() << "SQLiteAircraftDao::insertAircraft: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode() << "- flight ID" << flightId << "- sequence number" << sequenceNumber;
#endif
    }

    return aircraftId;
}

inline bool SQLiteAircraftDao::insertAircraftData(std::int64_t aircraftId, const Aircraft &aircraft) const noexcept
{
    bool ok {true};
    for (const auto &data : aircraft.getPosition()) {
        ok = d->positionDao->add(aircraftId, data);
        if (!ok) {
            break;
        }
    }
    if (ok) {
        for (const auto &data : aircraft.getAttitude()) {
            ok = d->attitudeDao->add(aircraftId, data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const auto &data : aircraft.getEngine()) {
            ok = d->engineDao->add(aircraftId, data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const auto &data : aircraft.getPrimaryFlightControl()) {
            ok = d->primaryFlightControlDao->add(aircraftId, data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const auto &data : aircraft.getSecondaryFlightControl()) {
            ok = d->secondaryFlightControlDao->add(aircraftId, data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const auto &data : aircraft.getAircraftHandle()) {
            ok = d->handleDao->add(aircraftId, data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const auto &data : aircraft.getLight()) {
            ok = d->lightDao->add(aircraftId, data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        ok = d->waypointDao->add(aircraftId, aircraft.getFlightPlan());
    }
    return ok;
}
