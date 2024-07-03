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
#include <vector>
#include <cstdint>
#include <utility>

#include <QString>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QTimeZone>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Model/FlightData.h>
#include <Model/FlightSummary.h>
#include <Model/FlightCondition.h>
#include "../../Dao/AircraftDaoIntf.h"
#include "../../Dao/DaoFactory.h"
#include "SQLiteFlightDao.h"
#include "SQLiteFlightDao.h"

struct SQLiteFlightDaoPrivate
{
    SQLiteFlightDaoPrivate(QString connectionName) noexcept
        : connectionName(connectionName),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite, std::move(connectionName))),
          aircraftDao(daoFactory->createAircraftDao())
    {}

    QString connectionName;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftDaoIntf> aircraftDao;
};

// PUBLIC

SQLiteFlightDao::SQLiteFlightDao(QString connectionName) noexcept
    : d {std::make_unique<SQLiteFlightDaoPrivate>(std::move(connectionName))}
{}

SQLiteFlightDao::SQLiteFlightDao(SQLiteFlightDao &&rhs) noexcept = default;
SQLiteFlightDao &SQLiteFlightDao::operator=(SQLiteFlightDao &&rhs) noexcept = default;
SQLiteFlightDao::~SQLiteFlightDao() = default;

bool SQLiteFlightDao::add(FlightData &flightData) const noexcept
{
    bool ok {false};
    const std::int64_t flightId = insertFlight(flightData);
    if (flightId != Const::InvalidId) {
        flightData.id = flightId;
        ok = addAircraft(flightId, flightData);
    }
    return ok;
}

bool SQLiteFlightDao::exportFlightData(const FlightData &flightData) const noexcept
{
    bool ok {false};
    const std::int64_t flightId = insertFlight(flightData);
    if (flightId != Const::InvalidId) {
        ok = exportAircraft(flightId, flightData);
    }
    return ok;
}

bool SQLiteFlightDao::get(std::int64_t id, FlightData &flightData) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from flight f "
        "where f.id = :id;"
    );

    query.bindValue(":id", QVariant::fromValue(id));
    bool ok = query.exec();
    if (ok) {
        flightData.clear(false, FlightData::CreationTimeMode::Reset);
        QSqlRecord record = query.record();
        const auto idIdx = record.indexOf("id");
        const auto creationTimeIdx = record.indexOf("creation_time");
        const auto userAircraftSequenceNumberIdx = record.indexOf("user_aircraft_seq_nr");
        const auto titleIdx = record.indexOf("title");
        const auto descriptionIdx = record.indexOf("description");
        const auto flightNumberIdx = record.indexOf("flight_number");
        const auto surfaceTypeIdx = record.indexOf("surface_type");
        const auto surfaceConditionIdx = record.indexOf("surface_condition");
        const auto onAnyRunwayIdx = record.indexOf("on_any_runway");
        const auto onParkingSpotIdx = record.indexOf("on_parking_spot");
        const auto groundAltitudeIdx = record.indexOf("ground_altitude");
        const auto ambientTemperatureIdx = record.indexOf("ambient_temperature");
        const auto totalAirTemperatureIdx = record.indexOf("total_air_temperature");
        const auto windSpeedIdx = record.indexOf("wind_speed");
        const auto windDirectionIdx = record.indexOf("wind_direction");
        const auto visibilityIdx = record.indexOf("visibility");
        const auto seaLevelPressureIdx = record.indexOf("sea_level_pressure");
        const auto pitotIcingIdx = record.indexOf("pitot_icing");
        const auto structuralIcingIdx = record.indexOf("structural_icing");
        const auto precipitationStateIdx = record.indexOf("precipitation_state");
        const auto inCloudsIdx = record.indexOf("in_clouds");
        const auto startLocalSimulationTimeIdx = record.indexOf("start_local_sim_time");
        const auto startZuluSimulationTimeIdx = record.indexOf("start_zulu_sim_time");
        const auto endLocalSimulationTimeIdx = record.indexOf("end_local_sim_time");
        const auto endZuluSimulationTimeIdx = record.indexOf("end_zulu_sim_time");

        if (query.next()) {
            flightData.id = query.value(idIdx).toLongLong();
            QDateTime dateTime = query.value(creationTimeIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            flightData.creationTime = dateTime.toLocalTime();
            flightData.title = query.value(titleIdx).toString();
            flightData.description = query.value(descriptionIdx).toString();
            flightData.flightNumber = query.value(flightNumberIdx).toString();

            FlightCondition &flightCondition = flightData.flightCondition;
            auto enumValue = query.value(surfaceTypeIdx).toInt();
            flightCondition.surfaceType = Enum::contains<SimType::SurfaceType>(enumValue) ? static_cast<SimType::SurfaceType>(enumValue) : SimType::SurfaceType::First;
            enumValue = query.value(surfaceConditionIdx).toInt();
            flightCondition.surfaceCondition = Enum::contains<SimType::SurfaceCondition>(enumValue) ? static_cast<SimType::SurfaceCondition>(enumValue) : SimType::SurfaceCondition::First;
            flightCondition.onAnyRunway = query.value(onAnyRunwayIdx).toBool();
            flightCondition.onParkingSpot = query.value(onParkingSpotIdx).toBool();            
            flightCondition.groundAltitude = query.value(groundAltitudeIdx).toFloat();
            flightCondition.ambientTemperature = query.value(ambientTemperatureIdx).toFloat();
            flightCondition.totalAirTemperature = query.value(totalAirTemperatureIdx).toFloat();
            flightCondition.windSpeed = query.value(windSpeedIdx).toFloat();
            flightCondition.windDirection = query.value(windDirectionIdx).toFloat();
            flightCondition.visibility = query.value(visibilityIdx).toFloat();
            flightCondition.seaLevelPressure = query.value(seaLevelPressureIdx).toFloat();
            flightCondition.pitotIcingPercent = query.value(pitotIcingIdx).toInt();
            flightCondition.structuralIcingPercent = query.value(structuralIcingIdx).toInt();
            enumValue = query.value(precipitationStateIdx).toInt();
            flightCondition.precipitationState = Enum::contains<SimType::PrecipitationState>(enumValue) ? static_cast<SimType::PrecipitationState>(enumValue) : SimType::PrecipitationState::First;
            flightCondition.inClouds = query.value(inCloudsIdx).toBool();
            // Persisted times is are already local respectively zulu simulation times
            flightCondition.startLocalDateTime = query.value(startLocalSimulationTimeIdx).toDateTime();
            flightCondition.startZuluDateTime = query.value(startZuluSimulationTimeIdx).toDateTime();
            flightCondition.endLocalDateTime = query.value(endLocalSimulationTimeIdx).toDateTime();
            flightCondition.endZuluDateTime = query.value(endZuluSimulationTimeIdx).toDateTime();
        }
        std::vector<Aircraft> aircraft = d->aircraftDao->getByFlightId(id, &ok);
        flightData.aircraft = std::move(aircraft);
        if (ok) {
            // Index starts at 0
            flightData.userAircraftIndex = query.value(userAircraftSequenceNumberIdx).toInt() - 1;
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteFlightDao::get: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool SQLiteFlightDao::deleteById(std::int64_t id) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from flight "
        "where id = :id;"
    );

    bool ok = d->aircraftDao->deleteAllByFlightId(id);
    if (ok) {
        query.bindValue(":id", QVariant::fromValue(id));
        ok = query.exec();
#ifdef DEBUG
        if (!ok) {
            qDebug() << "SQLiteFlightDao::deleteById: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
        }
#endif
    }
    return ok;
}

bool SQLiteFlightDao::updateTitle(std::int64_t id, const QString &title) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update flight "
        "set    title = :title "
        "where id = :id;"
    );

    query.bindValue(":title", title);
    query.bindValue(":id", QVariant::fromValue(id));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteFlightDao::updateTitleQuery: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteFlightDao::updateFlightNumber(std::int64_t id, const QString &flightNumber) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update flight "
        "set    flight_number = :flight_number "
        "where id = :id;"
    );

    query.bindValue(":flight_number", flightNumber);
    query.bindValue(":id", QVariant::fromValue(id));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteFlightDao::updateFlightNumber: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteFlightDao::updateDescription(std::int64_t id, const QString &description) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update flight "
        "set    description = :description "
        "where id = :id;"
    );

    query.bindValue(":description", description);
    query.bindValue(":id", QVariant::fromValue(id));
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteFlightDao::updateDescription: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteFlightDao::updateUserAircraftIndex(std::int64_t id, int index) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update flight "
        "set    user_aircraft_seq_nr = :user_aircraft_seq_nr "
        "where id = :id;"
    );

    // Sequence number starts at 1
    query.bindValue(":user_aircraft_seq_nr", index + 1);
    query.bindValue(":id", QVariant::fromValue(id));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteFlightDao::updateUserAircraftIndex: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

inline std::int64_t SQLiteFlightDao::insertFlight(const FlightData &flightData) const noexcept
{
    std::int64_t flightId {Const::InvalidId};

    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "insert into flight ("
        "  creation_time,"
        "  user_aircraft_seq_nr,"
        "  title,"
        "  description,"
        "  flight_number,"
        "  surface_type,"
        "  surface_condition,"
        "  on_any_runway,"
        "  on_parking_spot,"
        "  ground_altitude,"
        "  ambient_temperature,"
        "  total_air_temperature,"
        "  wind_speed,"
        "  wind_direction,"
        "  visibility,"
        "  sea_level_pressure,"
        "  pitot_icing,"
        "  structural_icing,"
        "  precipitation_state,"
        "  in_clouds,"
        "  start_local_sim_time,"
        "  start_zulu_sim_time,"
        "  end_local_sim_time,"
        "  end_zulu_sim_time"
        ") values ("
        " :creation_time,"
        " :user_aircraft_seq_nr,"
        " :title,"
        " :description,"
        " :flight_number,"
        " :surface_type,"
        " :surface_condition,"
        " :on_any_runway,"
        " :on_parking_spot,"
        " :ground_altitude,"
        " :ambient_temperature,"
        " :total_air_temperature,"
        " :wind_speed,"
        " :wind_direction,"
        " :visibility,"
        " :sea_level_pressure,"
        " :pitot_icing,"
        " :structural_icing,"
        " :precipitation_state,"
        " :in_clouds,"
        " :start_local_sim_time,"
        " :start_zulu_sim_time,"
        " :end_local_sim_time,"
        " :end_zulu_sim_time"
        ");"
    );

    const FlightCondition &flightCondition = flightData.flightCondition;
    query.bindValue(":creation_time", flightData.creationTime.toUTC());
    // Sequence number starts at 1
    query.bindValue(":user_aircraft_seq_nr", flightData.userAircraftIndex + 1);
    query.bindValue(":title", flightData.title);
    query.bindValue(":description", flightData.description);
    query.bindValue(":flight_number", flightData.flightNumber);
    query.bindValue(":surface_type", Enum::underly(flightCondition.surfaceType));
    query.bindValue(":surface_condition", Enum::underly(flightCondition.surfaceCondition));
    query.bindValue(":on_any_runway", flightCondition.onAnyRunway);
    query.bindValue(":on_parking_spot", flightCondition.onParkingSpot);
    query.bindValue(":ground_altitude", flightCondition.groundAltitude);
    query.bindValue(":ambient_temperature", flightCondition.ambientTemperature);
    query.bindValue(":total_air_temperature", flightCondition.totalAirTemperature);
    query.bindValue(":wind_speed", flightCondition.windSpeed);
    query.bindValue(":wind_direction", flightCondition.windDirection);
    query.bindValue(":visibility", flightCondition.visibility);
    query.bindValue(":sea_level_pressure", flightCondition.seaLevelPressure);
    query.bindValue(":pitot_icing", flightCondition.pitotIcingPercent);
    query.bindValue(":structural_icing", flightCondition.structuralIcingPercent);
    query.bindValue(":precipitation_state", Enum::underly(flightCondition.precipitationState));
    query.bindValue(":in_clouds", flightCondition.inClouds);
    // No conversion to UTC
    query.bindValue(":start_local_sim_time", flightCondition.startLocalDateTime);
    // Zulu time equals to UTC time
    query.bindValue(":start_zulu_sim_time", flightCondition.startZuluDateTime);
    // No conversion to UTC
    query.bindValue(":end_local_sim_time", flightCondition.endLocalDateTime);
    // Zulu time equals to UTC time
    query.bindValue(":end_zulu_sim_time", flightCondition.endZuluDateTime);
    bool ok = query.exec();
    if (ok) {
        flightId = query.lastInsertId().toLongLong(&ok);
    } else {
        flightId = Const::InvalidId;
#ifdef DEBUG
        qDebug() << "SQLiteFlightDao::insertFlight: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    return flightId;
}

inline bool SQLiteFlightDao::addAircraft(std::int64_t flightId, FlightData &flightData) const noexcept
{
    bool ok {true};
    // Starts at 1
    std::size_t sequenceNumber {1};
    for (auto &aircaft : flightData) {
        ok = d->aircraftDao->add(flightId, sequenceNumber, aircaft);
        if (ok) {
            ++sequenceNumber;
        } else {
            break;
        }
    }
    return ok;
}

inline bool SQLiteFlightDao::exportAircraft(std::int64_t flightId, const FlightData &flightData) const noexcept
{
    bool ok {true};
    // Starts at 1
    std::size_t sequenceNumber {1};
    for (const auto &aircaft : flightData) {
        ok = d->aircraftDao->exportAircraft(flightId, sequenceNumber, aircaft);
        if (ok) {
            ++sequenceNumber;
        } else {
            break;
        }
    }
    return ok;
}
