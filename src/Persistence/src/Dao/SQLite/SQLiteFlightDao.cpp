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
#include <QStringLiteral>
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
    : d(std::make_unique<SQLiteFlightDaoPrivate>(std::move(connectionName)))
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
    query.prepare(QStringLiteral(
        "select * "
        "from flight f "
        "where f.id = :id;"
    ));

    query.bindValue(QStringLiteral(":id"), QVariant::fromValue(id));
    bool ok = query.exec();
    if (ok) {
        flightData.clear(false, FlightData::CreationTimeMode::Reset);
        QSqlRecord record = query.record();
        const int idIdx = record.indexOf(QStringLiteral("id"));
        const int creationTimeIdx = record.indexOf(QStringLiteral("creation_time"));
        const int userAircraftSequenceNumberIdx = record.indexOf(QStringLiteral("user_aircraft_seq_nr"));
        const int titleIdx = record.indexOf(QStringLiteral("title"));
        const int descriptionIdx = record.indexOf(QStringLiteral("description"));
        const int flightNumberIdx = record.indexOf(QStringLiteral("flight_number"));
        const int surfaceTypeIdx = record.indexOf(QStringLiteral("surface_type"));
        const int surfaceConditionIdx = record.indexOf(QStringLiteral("surface_condition"));
        const int onAnyRunwayIdx = record.indexOf(QStringLiteral("on_any_runway"));
        const int onParkingSpotIdx = record.indexOf(QStringLiteral("on_parking_spot"));
        const int groundAltitudeIdx = record.indexOf(QStringLiteral("ground_altitude"));
        const int ambientTemperatureIdx = record.indexOf(QStringLiteral("ambient_temperature"));
        const int totalAirTemperatureIdx = record.indexOf(QStringLiteral("total_air_temperature"));
        const int windSpeedIdx = record.indexOf(QStringLiteral("wind_speed"));
        const int windDirectionIdx = record.indexOf(QStringLiteral("wind_direction"));
        const int visibilityIdx = record.indexOf(QStringLiteral("visibility"));
        const int seaLevelPressureIdx = record.indexOf(QStringLiteral("sea_level_pressure"));
        const int pitotIcingIdx = record.indexOf(QStringLiteral("pitot_icing"));
        const int structuralIcingIdx = record.indexOf(QStringLiteral("structural_icing"));
        const int precipitationStateIdx = record.indexOf(QStringLiteral("precipitation_state"));
        const int inCloudsIdx = record.indexOf(QStringLiteral("in_clouds"));
        const int startLocalSimulationTimeIdx = record.indexOf(QStringLiteral("start_local_sim_time"));
        const int startZuluSimulationTimeIdx = record.indexOf(QStringLiteral("start_zulu_sim_time"));
        const int endLocalSimulationTimeIdx = record.indexOf(QStringLiteral("end_local_sim_time"));
        const int endZuluSimulationTimeIdx = record.indexOf(QStringLiteral("end_zulu_sim_time"));

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
    query.prepare(QStringLiteral(
        "delete "
        "from flight "
        "where id = :id;"
    ));

    bool ok = d->aircraftDao->deleteAllByFlightId(id);
    if (ok) {
        query.bindValue(QStringLiteral(":id"), QVariant::fromValue(id));
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
    query.prepare(QStringLiteral(
        "update flight "
        "set    title = :title "
        "where id = :id;"
    ));

    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(QStringLiteral(":id"), QVariant::fromValue(id));
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
    query.prepare(QStringLiteral(
        "update flight "
        "set    flight_number = :flight_number "
        "where id = :id;"
    ));

    query.bindValue(QStringLiteral(":flight_number"), flightNumber);
    query.bindValue(QStringLiteral(":id"), QVariant::fromValue(id));
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
    query.prepare(QStringLiteral(
        "update flight "
        "set    description = :description "
        "where id = :id;"
    ));

    query.bindValue(QStringLiteral(":description"), description);
    query.bindValue(QStringLiteral(":id"), QVariant::fromValue(id));
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
    query.prepare(QStringLiteral(
        "update flight "
        "set    user_aircraft_seq_nr = :user_aircraft_seq_nr "
        "where id = :id;"
    ));

    // Sequence number starts at 1
    query.bindValue(QStringLiteral(":user_aircraft_seq_nr"), index + 1);
    query.bindValue(QStringLiteral(":id"), QVariant::fromValue(id));
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
    query.prepare(QStringLiteral(
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
    ));

    const FlightCondition &flightCondition = flightData.flightCondition;
    query.bindValue(QStringLiteral(":creation_time"), flightData.creationTime.toUTC());
    // Sequence number starts at 1
    query.bindValue(QStringLiteral(":user_aircraft_seq_nr"), flightData.userAircraftIndex + 1);
    query.bindValue(QStringLiteral(":title"), flightData.title);
    query.bindValue(QStringLiteral(":description"), flightData.description);
    query.bindValue(QStringLiteral(":flight_number"), flightData.flightNumber);
    query.bindValue(QStringLiteral(":surface_type"), Enum::underly(flightCondition.surfaceType));
    query.bindValue(QStringLiteral(":surface_condition"), Enum::underly(flightCondition.surfaceCondition));
    query.bindValue(QStringLiteral(":on_any_runway"), flightCondition.onAnyRunway);
    query.bindValue(QStringLiteral(":on_parking_spot"), flightCondition.onParkingSpot);
    query.bindValue(QStringLiteral(":ground_altitude"), flightCondition.groundAltitude);
    query.bindValue(QStringLiteral(":ambient_temperature"), flightCondition.ambientTemperature);
    query.bindValue(QStringLiteral(":total_air_temperature"), flightCondition.totalAirTemperature);
    query.bindValue(QStringLiteral(":wind_speed"), flightCondition.windSpeed);
    query.bindValue(QStringLiteral(":wind_direction"), flightCondition.windDirection);
    query.bindValue(QStringLiteral(":visibility"), flightCondition.visibility);
    query.bindValue(QStringLiteral(":sea_level_pressure"), flightCondition.seaLevelPressure);
    query.bindValue(QStringLiteral(":pitot_icing"), flightCondition.pitotIcingPercent);
    query.bindValue(QStringLiteral(":structural_icing"), flightCondition.structuralIcingPercent);
    query.bindValue(QStringLiteral(":precipitation_state"), Enum::underly(flightCondition.precipitationState));
    query.bindValue(QStringLiteral(":in_clouds"), flightCondition.inClouds);
    // No conversion to UTC
    query.bindValue(QStringLiteral(":start_local_sim_time"), flightCondition.startLocalDateTime);
    // Zulu time equals to UTC time
    query.bindValue(QStringLiteral(":start_zulu_sim_time"), flightCondition.startZuluDateTime);
    // No conversion to UTC
    query.bindValue(QStringLiteral(":end_local_sim_time"), flightCondition.endLocalDateTime);
    // Zulu time equals to UTC time
    query.bindValue(QStringLiteral(":end_zulu_sim_time"), flightCondition.endZuluDateTime);
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
