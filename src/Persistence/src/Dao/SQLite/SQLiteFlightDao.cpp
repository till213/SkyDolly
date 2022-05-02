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
#include <vector>
#include <iterator>
#include <cstdint>

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QTimeZone>

#include <Kernel/Enum.h>
#include <Model/Flight.h>
#include <Model/FlightSummary.h>
#include <Model/FlightCondition.h>
#include "../../Dao/AircraftDaoIntf.h"
#include "../../Dao/DaoFactory.h"
#include <ConnectionManager.h>
#include "SQLiteFlightDao.h"
#include "SQLiteFlightDao.h"

class SQLiteFlightDaoPrivate
{
public:
    SQLiteFlightDaoPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          aircraftDao(daoFactory->createAircraftDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftDaoIntf> aircraftDao;
};

// PUBLIC

SQLiteFlightDao::SQLiteFlightDao() noexcept
    : d(std::make_unique<SQLiteFlightDaoPrivate>())
{}

SQLiteFlightDao::~SQLiteFlightDao() noexcept
{}

bool SQLiteFlightDao::addFlight(Flight &flight) noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into flight ("
        "  creation_time,"
        "  title,"
        "  description,"
        "  user_aircraft_seq_nr,"
        "  surface_type,"
        "  ground_altitude,"
        "  ambient_temperature,"
        "  total_air_temperature,"
        "  wind_velocity,"
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
        " :title,"
        " :description,"
        " :user_aircraft_seq_nr,"
        " :surface_type,"
        " :ground_altitude,"
        " :ambient_temperature,"
        " :total_air_temperature,"
        " :wind_velocity,"
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

    const FlightCondition &flightCondition = flight.getFlightConditionConst();
    query.bindValue(":creation_time", flight.getCreationTime().toUTC());
    query.bindValue(":title", flight.getTitle());
    query.bindValue(":description", flight.getDescription());
    // Sequence number starts at 1
    query.bindValue(":user_aircraft_seq_nr", flight.getUserAircraftIndex() + 1);
    query.bindValue(":surface_type", Enum::toUnderlyingType(flightCondition.surfaceType));
    query.bindValue(":ground_altitude", flightCondition.groundAltitude);
    query.bindValue(":ambient_temperature", flightCondition.ambientTemperature);
    query.bindValue(":total_air_temperature", flightCondition.totalAirTemperature);
    query.bindValue(":wind_velocity", flightCondition.windVelocity);
    query.bindValue(":wind_direction", flightCondition.windDirection);
    query.bindValue(":visibility", flightCondition.visibility);
    query.bindValue(":sea_level_pressure", flightCondition.seaLevelPressure);
    query.bindValue(":pitot_icing", flightCondition.pitotIcingPercent);
    query.bindValue(":structural_icing", flightCondition.structuralIcingPercent);
    query.bindValue(":precipitation_state", Enum::toUnderlyingType(flightCondition.precipitationState));
    query.bindValue(":in_clouds", flightCondition.inClouds);
    // No conversion to UTC
    query.bindValue(":start_local_sim_time", flightCondition.startLocalTime);
    // Zulu time equals to UTC time
    query.bindValue(":start_zulu_sim_time", flightCondition.startZuluTime);
    // No conversion to UTC
    query.bindValue(":end_local_sim_time", flightCondition.endLocalTime);
    // Zulu time equals to UTC time
    query.bindValue(":end_zulu_sim_time", flightCondition.endZuluTime);
    bool ok = query.exec();
    if (ok) {
        std::int64_t id = query.lastInsertId().toLongLong(&ok);
        flight.setId(id);
#ifdef DEBUG
    } else {
        qDebug("SQLiteFlightDao::addFlight: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }
    if (ok) {
        // Starts at 1
        int sequenceNumber = 1;
        for (auto &it : flight) {
            ok = d->aircraftDao->add(flight.getId(), sequenceNumber, *it.get());
            if (ok) {
                ++sequenceNumber;
            } else {
                break;
            }
        }

    }
    return ok;
}

bool SQLiteFlightDao::getFlightById(std::int64_t id, Flight &flight) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from flight f "
        "where f.id = :id;"
    );

    query.bindValue(":id", QVariant::fromValue(id));
    bool ok = query.exec();
    if (ok) {
        flight.clear(false);
        QSqlRecord record = query.record();
        const int idIdx = record.indexOf("id");
        const int creationTimeIdx = record.indexOf("creation_time");
        const int titleIdx = record.indexOf("title");
        const int descriptionIdx = record.indexOf("description");
        const int surfaceTypeIdx = record.indexOf("surface_type");
        const int groundAltitudeIdx = record.indexOf("ground_altitude");
        const int ambientTemperatureIdx = record.indexOf("ambient_temperature");
        const int totalAirTemperatureIdx = record.indexOf("total_air_temperature");
        const int windVelocityIdx = record.indexOf("wind_velocity");
        const int windDirectionIdx = record.indexOf("wind_direction");
        const int visibilityIdx = record.indexOf("visibility");
        const int seaLevelPressureIdx = record.indexOf("sea_level_pressure");
        const int pitotIcingIdx = record.indexOf("pitot_icing");
        const int structuralIcingIdx = record.indexOf("structural_icing");
        const int precipitationStateIdx = record.indexOf("precipitation_state");
        const int inCloudsIdx = record.indexOf("in_clouds");
        const int startLocalSimulationTimeIdx = record.indexOf("start_local_sim_time");
        const int startZuluSimulationTimeIdx = record.indexOf("start_zulu_sim_time");
        const int endLocalSimulationTimeIdx = record.indexOf("end_local_sim_time");
        const int endZuluSimulationTimeIdx = record.indexOf("end_zulu_sim_time");
        const int userAircraftSequenceNumberIdx = record.indexOf("user_aircraft_seq_nr");
        if (query.next()) {
            flight.setId(query.value(idIdx).toLongLong());
            QDateTime dateTime = query.value(creationTimeIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            flight.setCreationTime(dateTime.toLocalTime());
            flight.setTitle(query.value(titleIdx).toString());
            flight.setDescription(query.value(descriptionIdx).toString());

            FlightCondition flightCondition;
            flightCondition.surfaceType = static_cast<SimType::SurfaceType>(query.value(surfaceTypeIdx).toInt());
            flightCondition.groundAltitude = query.value(groundAltitudeIdx).toFloat();
            flightCondition.ambientTemperature = query.value(ambientTemperatureIdx).toFloat();
            flightCondition.totalAirTemperature = query.value(totalAirTemperatureIdx).toFloat();
            flightCondition.windVelocity = query.value(windVelocityIdx).toFloat();
            flightCondition.windDirection = query.value(windDirectionIdx).toFloat();
            flightCondition.visibility = query.value(visibilityIdx).toFloat();
            flightCondition.seaLevelPressure = query.value(seaLevelPressureIdx).toFloat();
            flightCondition.pitotIcingPercent = query.value(pitotIcingIdx).toInt();
            flightCondition.structuralIcingPercent = query.value(structuralIcingIdx).toInt();
            flightCondition.precipitationState = static_cast<SimType::PrecipitationState>(query.value(precipitationStateIdx).toInt());
            flightCondition.inClouds = query.value(inCloudsIdx).toBool();
            // Persisted times is are already local respectively zulu simulation times
            flightCondition.startLocalTime = query.value(startLocalSimulationTimeIdx).toDateTime();
            flightCondition.startZuluTime = query.value(startZuluSimulationTimeIdx).toDateTime();
            flightCondition.endLocalTime = query.value(endLocalSimulationTimeIdx).toDateTime();
            flightCondition.endZuluTime = query.value(endZuluSimulationTimeIdx).toDateTime();

            flight.setFlightCondition(flightCondition);
        }
        std::vector<std::unique_ptr<Aircraft>> aircraft;
        ok = d->aircraftDao->getByFlightId(id, std::back_inserter(aircraft));
        flight.setAircraft(std::move(aircraft));
        if (ok) {
            // Index starts at 0
            const int userAircraftIndex = query.value(userAircraftSequenceNumberIdx).toInt() - 1;
            flight.setUserAircraftIndex(userAircraftIndex);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteFlightDao::getFlightById: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }
    return ok;
}

bool SQLiteFlightDao::deleteById(std::int64_t id) noexcept
{
    QSqlQuery query;
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
            qDebug("SQLiteFlightDao::deleteById: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
        }
#endif
    }
    return ok;
}

bool SQLiteFlightDao::updateTitle(std::int64_t id, const QString &title) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update flight "
        "set    title = :title "
        "where id = :id;"
    );

    query.bindValue(":title", title);
    query.bindValue(":id", QVariant::fromValue(id));
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteFlightDao::updateTitleQuery: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteFlightDao::updateTitleAndDescription(std::int64_t id, const QString &title, const QString &description) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update flight "
        "set    title = :title,"
        "       description = :description "
        "where id = :id;"
    );

    query.bindValue(":title", title);
    query.bindValue(":description", description);
    query.bindValue(":id", QVariant::fromValue(id));
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteFlightDao::updateTitleAndDescription: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteFlightDao::updateUserAircraftIndex(std::int64_t id, int index) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update flight "
        "set    user_aircraft_seq_nr = :user_aircraft_seq_nr "
        "where id = :id;"
    );

    // Sequence number starts at 1
    query.bindValue(":user_aircraft_seq_nr", index + 1);
    query.bindValue(":id", QVariant::fromValue(id));
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteFlightDao::updateUserAircraftIndex: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}
