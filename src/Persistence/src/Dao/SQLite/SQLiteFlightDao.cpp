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
#include <memory>

#include <QObject>
#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QVector>
#include <QDateTime>
#include <QTimeZone>

#include "../../../../Kernel/src/Enum.h"
#include "../../../../Model/src/Flight.h"
#include "../../../../Model/src/FlightSummary.h"
#include "../../../../Model/src/FlightCondition.h"
#include "../../Dao/AircraftDaoIntf.h"
#include "../../Dao/DaoFactory.h"
#include "../../ConnectionManager.h"
#include "SQLiteFlightDao.h"
#include "SQLiteFlightDao.h"

class SQLiteFlightDaoPrivate
{
public:
    SQLiteFlightDaoPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          aircraftDao(daoFactory->createAircraftDao())
    {}

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByIdQuery;
    std::unique_ptr<QSqlQuery> removeByIdQuery;
    std::unique_ptr<QSqlQuery> updateTitleQuery;
    std::unique_ptr<QSqlQuery> updateTitleAndDescriptionQuery;
    std::unique_ptr<QSqlQuery> updateUserAircraftSequenceNumberQuery;
    std::unique_ptr<QSqlQuery> selectSummariesQuery;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftDaoIntf> aircraftDao;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();
            insertQuery->prepare(
"insert into flight ("
"  id,"
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
"  null,"
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
");");
        }
        if (selectByIdQuery == nullptr) {
            selectByIdQuery = std::make_unique<QSqlQuery>();
            selectByIdQuery->setForwardOnly(true);
            selectByIdQuery->prepare(
"select * "
"from flight f "
"where f.id = :id;");
        }
        if (removeByIdQuery == nullptr) {
            removeByIdQuery = std::make_unique<QSqlQuery>();
            removeByIdQuery->prepare(
"delete "
"from flight "
"where id = :id;");
        }
        if (updateTitleQuery == nullptr) {
            updateTitleQuery = std::make_unique<QSqlQuery>();
            updateTitleQuery->prepare(
"update flight "
"set    title = :title "
"where id = :id;");
        }
        if (updateTitleAndDescriptionQuery == nullptr) {
            updateTitleAndDescriptionQuery = std::make_unique<QSqlQuery>();
            updateTitleAndDescriptionQuery->prepare(
"update flight "
"set    title = :title,"
"       description = :description "
"where id = :id;");
        }
        if (updateUserAircraftSequenceNumberQuery == nullptr) {
            updateUserAircraftSequenceNumberQuery = std::make_unique<QSqlQuery>();
            updateUserAircraftSequenceNumberQuery->prepare(
"update flight "
"set    user_aircraft_seq_nr = :user_aircraft_seq_nr "
"where id = :id;");
        }
        if (selectSummariesQuery == nullptr) {
            selectSummariesQuery = std::make_unique<QSqlQuery>();
            selectSummariesQuery->setForwardOnly(true);
            selectSummariesQuery->prepare(
"select f.id, f.creation_date, f.title, a.type, (select count(*) from aircraft where aircraft.flight_id = f.id) as aircraft_count, "
"       a.start_date, f.start_local_sim_time, f.start_zulu_sim_time, fp1.ident as start_waypoint,"
"       a.end_date, f.end_local_sim_time, f.end_zulu_sim_time, fp2.ident as end_waypoint "
"from   flight f "
"join   aircraft a "
"on     a.flight_id = f.id "
"and    a.seq_nr = 1 "
"left join (select ident, aircraft_id from waypoint wo1 where wo1.timestamp = (select min(wi1.timestamp) from waypoint wi1 where wi1.aircraft_id = wo1.aircraft_id)) fp1 "
"on fp1.aircraft_id = a.id "
"left join (select ident, aircraft_id from waypoint wo2 where wo2.timestamp = (select max(wi2.timestamp) from waypoint wi2 where wi2.aircraft_id = wo2.aircraft_id)) fp2 "
"on fp2.aircraft_id = a.id");
        }
    }

    void resetQueries() noexcept
    {
        insertQuery = nullptr;
        selectByIdQuery = nullptr;
        removeByIdQuery = nullptr;
        updateTitleQuery = nullptr;
        updateTitleAndDescriptionQuery = nullptr;
        updateUserAircraftSequenceNumberQuery = nullptr;
        selectSummariesQuery = nullptr;
    }
};

// PUBLIC

SQLiteFlightDao::SQLiteFlightDao(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SQLiteFlightDaoPrivate>())
{
    frenchConnection();
}

SQLiteFlightDao::~SQLiteFlightDao() noexcept
{}

bool SQLiteFlightDao::addFlight(Flight &flight) noexcept
{
    d->initQueries();
    const FlightCondition &flightCondition = flight.getFlightConditionConst();
    d->insertQuery->bindValue(":title", flight.getTitle());
    d->insertQuery->bindValue(":description", flight.getDescription());
    // Sequence number starts at 1
    d->insertQuery->bindValue(":user_aircraft_seq_nr", flight.getUserAircraftIndex() + 1);
    d->insertQuery->bindValue(":surface_type", Enum::toUnderlyingType(flightCondition.surfaceType));
    d->insertQuery->bindValue(":ground_altitude", flightCondition.groundAltitude);
    d->insertQuery->bindValue(":ambient_temperature", flightCondition.ambientTemperature);
    d->insertQuery->bindValue(":total_air_temperature", flightCondition.totalAirTemperature);
    d->insertQuery->bindValue(":wind_velocity", flightCondition.windVelocity);
    d->insertQuery->bindValue(":wind_direction", flightCondition.windDirection);
    d->insertQuery->bindValue(":visibility", flightCondition.visibility);
    d->insertQuery->bindValue(":sea_level_pressure", flightCondition.seaLevelPressure);
    d->insertQuery->bindValue(":pitot_icing", flightCondition.pitotIcingPercent);
    d->insertQuery->bindValue(":structural_icing", flightCondition.structuralIcingPercent);
    d->insertQuery->bindValue(":precipitation_state", Enum::toUnderlyingType(flightCondition.precipitationState));
    d->insertQuery->bindValue(":in_clouds", flightCondition.inClouds);
    // No conversion to UTC
    d->insertQuery->bindValue(":start_local_sim_time", flightCondition.startLocalTime);
    // Zulu time equals to UTC time
    d->insertQuery->bindValue(":start_zulu_sim_time", flightCondition.startZuluTime);
    // No conversion to UTC
    d->insertQuery->bindValue(":end_local_sim_time", flightCondition.endLocalTime);
    // Zulu time equals to UTC time
    d->insertQuery->bindValue(":end_zulu_sim_time", flightCondition.endZuluTime);   
    bool ok = d->insertQuery->exec();
    if (ok) {
        qint64 id = d->insertQuery->lastInsertId().toLongLong(&ok);
        flight.setId(id);
#ifdef DEBUG
    } else {
        qDebug("SQLiteFlightDao::addFlight: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
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

bool SQLiteFlightDao::getFlightById(qint64 id, Flight &flight) const noexcept
{
    d->initQueries();
    d->selectByIdQuery->bindValue(":id", id);
    bool ok = d->selectByIdQuery->exec();
    if (ok) {
        flight.clear();
        QSqlRecord record = d->selectByIdQuery->record();
        const int idIdx = record.indexOf("id");
        const int creationDateIdx = record.indexOf("creation_date");
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
        if (d->selectByIdQuery->next()) {
            flight.setId(d->selectByIdQuery->value(idIdx).toLongLong());
            QDateTime date = d->selectByIdQuery->value(creationDateIdx).toDateTime();
            date.setTimeZone(QTimeZone::utc());
            flight.setCreationDate(date.toLocalTime());
            flight.setTitle(d->selectByIdQuery->value(titleIdx).toString());
            flight.setDescription(d->selectByIdQuery->value(descriptionIdx).toString());

            FlightCondition flightCondition;
            flightCondition.surfaceType = static_cast<SimType::SurfaceType>(d->selectByIdQuery->value(surfaceTypeIdx).toInt());
            flightCondition.groundAltitude = d->selectByIdQuery->value(groundAltitudeIdx).toFloat();
            flightCondition.ambientTemperature = d->selectByIdQuery->value(ambientTemperatureIdx).toFloat();
            flightCondition.totalAirTemperature = d->selectByIdQuery->value(totalAirTemperatureIdx).toFloat();
            flightCondition.windVelocity = d->selectByIdQuery->value(windVelocityIdx).toFloat();
            flightCondition.windDirection = d->selectByIdQuery->value(windDirectionIdx).toFloat();
            flightCondition.visibility = d->selectByIdQuery->value(visibilityIdx).toFloat();
            flightCondition.seaLevelPressure = d->selectByIdQuery->value(seaLevelPressureIdx).toFloat();
            flightCondition.pitotIcingPercent = d->selectByIdQuery->value(pitotIcingIdx).toInt();
            flightCondition.structuralIcingPercent = d->selectByIdQuery->value(structuralIcingIdx).toInt();
            flightCondition.precipitationState = static_cast<SimType::PrecipitationState>(d->selectByIdQuery->value(precipitationStateIdx).toInt());
            flightCondition.inClouds = d->selectByIdQuery->value(inCloudsIdx).toBool();
            // Persisted times is are already local respectively zulu simulation times
            flightCondition.startLocalTime = d->selectByIdQuery->value(startLocalSimulationTimeIdx).toDateTime();
            flightCondition.startZuluTime = d->selectByIdQuery->value(startZuluSimulationTimeIdx).toDateTime();
            flightCondition.endLocalTime = d->selectByIdQuery->value(endLocalSimulationTimeIdx).toDateTime();
            flightCondition.endZuluTime = d->selectByIdQuery->value(endZuluSimulationTimeIdx).toDateTime();

            flight.setFlightCondition(flightCondition);
        }
        ok = d->aircraftDao->getByFlightId(id, flight.getAircrafts());
        if (ok) {
            // Index starts at 0
            const int userAircraftIndex = d->selectByIdQuery->value(userAircraftSequenceNumberIdx).toInt() - 1;
            flight.setUserAircraftIndex(userAircraftIndex);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteFlightDao::getFlightById: SQL error: %s", qPrintable(d->selectByIdQuery->lastError().databaseText() + " - error code: " + d->selectByIdQuery->lastError().nativeErrorCode()));
#endif
    }
    return ok;
}

bool SQLiteFlightDao::removeById(qint64 id) noexcept
{
    d->initQueries();

    bool ok = d->aircraftDao->removeAllByFlightId(id);
    if (ok) {
        d->removeByIdQuery->bindValue(":id", id);
        ok = d->removeByIdQuery->exec();
#ifdef DEBUG
        if (!ok) {
            qDebug("SQLiteFlightDao::removeById: SQL error: %s", qPrintable(d->removeByIdQuery->lastError().databaseText() + " - error code: " + d->removeByIdQuery->lastError().nativeErrorCode()));
        }
#endif
    }
    return ok;
}

bool SQLiteFlightDao::updateTitle(qint64 id, const QString &title) noexcept
{
    d->initQueries();

    d->updateTitleQuery->bindValue(":title", title);
    d->updateTitleQuery->bindValue(":id", id);
    bool ok = d->updateTitleQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteFlightDao::updateTitleQuery: SQL error: %s", qPrintable(d->updateTitleQuery->lastError().databaseText() + " - error code: " + d->updateTitleQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteFlightDao::updateTitleAndDescription(qint64 id, const QString &title, const QString &description) noexcept
{
    d->initQueries();

    d->updateTitleAndDescriptionQuery->bindValue(":title", title);
    d->updateTitleAndDescriptionQuery->bindValue(":description", description);
    d->updateTitleAndDescriptionQuery->bindValue(":id", id);
    bool ok = d->updateTitleAndDescriptionQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteFlightDao::updateTitleAndDescription: SQL error: %s", qPrintable(d->updateTitleAndDescriptionQuery->lastError().databaseText() + " - error code: " + d->updateTitleAndDescriptionQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteFlightDao::updateUserAircraftIndex(qint64 id, int index) noexcept
{
    d->initQueries();

    // Sequence number starts at 1
    d->updateUserAircraftSequenceNumberQuery->bindValue(":user_aircraft_seq_nr", index + 1);
    d->updateUserAircraftSequenceNumberQuery->bindValue(":id", id);
    bool ok = d->updateUserAircraftSequenceNumberQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteFlightDao::updateUserAircraftIndex: SQL error: %s", qPrintable(d->updateUserAircraftSequenceNumberQuery->lastError().databaseText() + " - error code: " + d->updateUserAircraftSequenceNumberQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

QVector<FlightSummary> SQLiteFlightDao::getFlightSummaries() const noexcept
{
    QVector<FlightSummary> summaries;

    d->initQueries();
    bool ok = d->selectSummariesQuery->exec();
    if (ok) {
        QSqlRecord record = d->selectSummariesQuery->record();
        const int idIdx = record.indexOf("id");
        const int creationDateIdx = record.indexOf("creation_date");
        const int aircraftTypeIdx = record.indexOf("type");
        const int aircraftCountIdx = record.indexOf("aircraft_count");
        const int startDateIdx = record.indexOf("start_date");
        const int startLocalSimulationTimeIdx = record.indexOf("start_local_sim_time");
        const int startZuluSimulationTimeIdx = record.indexOf("start_zulu_sim_time");
        const int startWaypointIdx = record.indexOf("start_waypoint");
        const int endDateIdx = record.indexOf("end_date");
        const int endLocalSimulationTimeIdx = record.indexOf("end_local_sim_time");
        const int endZuluSimulationTimeIdx = record.indexOf("end_zulu_sim_time");
        const int endWaypointIdx = record.indexOf("end_waypoint");
        const int titleIdx = record.indexOf("title");
        while (d->selectSummariesQuery->next()) {

            FlightSummary summary;
            summary.id = d->selectSummariesQuery->value(idIdx).toLongLong();

            QDateTime dateTime = d->selectSummariesQuery->value(creationDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            summary.creationDate = dateTime.toLocalTime();
            summary.aircraftType = d->selectSummariesQuery->value(aircraftTypeIdx).toString();
            summary.aircraftCount = d->selectSummariesQuery->value(aircraftCountIdx).toInt();
            dateTime = d->selectSummariesQuery->value(startDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            summary.startDate = dateTime.toLocalTime();
            // Persisted times is are already local respectively zulu simulation times
            summary.startSimulationLocalTime = d->selectSummariesQuery->value(startLocalSimulationTimeIdx).toDateTime();
            summary.startSimulationZuluTime = d->selectSummariesQuery->value(startZuluSimulationTimeIdx).toDateTime();
            summary.startLocation = d->selectSummariesQuery->value(startWaypointIdx).toString();
            dateTime = d->selectSummariesQuery->value(endDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            summary.endDate = dateTime.toLocalTime();
            // Persisted times is are already local respectively zulu simulation times
            summary.endSimulationLocalTime = d->selectSummariesQuery->value(endLocalSimulationTimeIdx).toDateTime();
            summary.endSimulationZuluTime = d->selectSummariesQuery->value(endZuluSimulationTimeIdx).toDateTime();
            summary.endLocation = d->selectSummariesQuery->value(endWaypointIdx).toString();
            summary.title = d->selectSummariesQuery->value(titleIdx).toString();

            summaries.append(summary);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteFlightDao::getFlightDescriptions: SQL error: %s", qPrintable(d->selectSummariesQuery->lastError().databaseText() + " - error code: " + d->selectSummariesQuery->lastError().nativeErrorCode()));
#endif
    }

    return summaries;
}

// PRIVATE

void SQLiteFlightDao::frenchConnection() noexcept
{
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &SQLiteFlightDao::handleConnectionChanged);
}

// PRIVATE SLOTS

void SQLiteFlightDao::handleConnectionChanged() noexcept
{
    d->resetQueries();
}
