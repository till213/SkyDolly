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

namespace  {
    constexpr int UserAircraftSequenceNumber = 1;
}

class SQLiteFlightDaoPrivate
{
public:
    SQLiteFlightDaoPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          aircraftDao(daoFactory->createAircraftDao())
    {}

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByIdQuery;
    std::unique_ptr<QSqlQuery> deleteByIdQuery;
    std::unique_ptr<QSqlQuery> updateTitleQuery;
    std::unique_ptr<QSqlQuery> updateTitleAndDescriptionQuery;
    std::unique_ptr<QSqlQuery> selectDescriptionsQuery;
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
        if (deleteByIdQuery == nullptr) {
            deleteByIdQuery = std::make_unique<QSqlQuery>();
            deleteByIdQuery->prepare(
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
        if (selectDescriptionsQuery == nullptr) {
            selectDescriptionsQuery = std::make_unique<QSqlQuery>();
            selectDescriptionsQuery->setForwardOnly(true);
            selectDescriptionsQuery->prepare(
"select f.id, f.creation_date, f.title, a.type,"
"       a.start_date, f.start_local_sim_time, f.start_zulu_sim_time, fp1.ident as start_waypoint,"
"       a.end_date, f.end_local_sim_time, f.end_zulu_sim_time, fp2.ident as end_waypoint "
"from   flight f "
"join   aircraft a "
"on     a.flight_id = f.id "
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
        deleteByIdQuery = nullptr;
        updateTitleQuery = nullptr;
        updateTitleAndDescriptionQuery = nullptr;
        selectDescriptionsQuery = nullptr;
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

bool SQLiteFlightDao::addFlight(Flight &flight)  noexcept
{
    d->initQueries();
    const FlightCondition &flightCondition = flight.getFlightConditionConst();
    d->insertQuery->bindValue(":title", flight.getTitle());
    d->insertQuery->bindValue(":description", flight.getDescription());
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
        const int idIdx = d->selectByIdQuery->record().indexOf("id");
        const int creationDateIdx = d->selectByIdQuery->record().indexOf("creation_date");
        const int titleIdx = d->selectByIdQuery->record().indexOf("title");
        const int descriptionIdx = d->selectByIdQuery->record().indexOf("description");
        const int surfaceTypeIdx = d->selectByIdQuery->record().indexOf("surface_type");
        const int groundAltitudeIdx = d->selectByIdQuery->record().indexOf("ground_altitude");
        const int ambientTemperatureIdx = d->selectByIdQuery->record().indexOf("ambient_temperature");
        const int totalAirTemperatureIdx = d->selectByIdQuery->record().indexOf("total_air_temperature");
        const int windVelocityIdx = d->selectByIdQuery->record().indexOf("wind_velocity");
        const int windDirectionIdx = d->selectByIdQuery->record().indexOf("wind_direction");
        const int visibilityIdx = d->selectByIdQuery->record().indexOf("visibility");
        const int seaLevelPressureIdx = d->selectByIdQuery->record().indexOf("sea_level_pressure");
        const int pitotIcingIdx = d->selectByIdQuery->record().indexOf("pitot_icing");
        const int structuralIcingIdx = d->selectByIdQuery->record().indexOf("structural_icing");
        const int precipitationStateIdx = d->selectByIdQuery->record().indexOf("precipitation_state");
        const int inCloudsIdx = d->selectByIdQuery->record().indexOf("in_clouds");
        const int startLocalSimulationTimeIdx = d->selectByIdQuery->record().indexOf("start_local_sim_time");
        const int startZuluSimulationTimeIdx = d->selectByIdQuery->record().indexOf("start_zulu_sim_time");
        const int endLocalSimulationTimeIdx = d->selectByIdQuery->record().indexOf("end_local_sim_time");
        const int endZuluSimulationTimeIdx = d->selectByIdQuery->record().indexOf("end_zulu_sim_time");
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
#ifdef DEBUG
    } else {
        qDebug("SQLiteFlightDao::getFlightById: SQL error: %s", qPrintable(d->selectByIdQuery->lastError().databaseText() + " - error code: " + d->selectByIdQuery->lastError().nativeErrorCode()));
#endif
    }
    return ok;
}

bool SQLiteFlightDao::deleteById(qint64 id) noexcept
{
    d->initQueries();

    bool ok = d->aircraftDao->deleteByFlightId(id);
    if (ok) {
        d->deleteByIdQuery->bindValue(":id", id);
        ok = d->deleteByIdQuery->exec();
#ifdef DEBUG
        if (!ok) {
            qDebug("SQLiteFlightDao::deleteById: SQL error: %s", qPrintable(d->deleteByIdQuery->lastError().databaseText() + " - error code: " + d->deleteByIdQuery->lastError().nativeErrorCode()));
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

QVector<FlightSummary> SQLiteFlightDao::getFlightSummaries() const noexcept
{
    QVector<FlightSummary> summaries;

    d->initQueries();
    bool ok = d->selectDescriptionsQuery->exec();
    if (ok) {
        const int idIdx = d->selectDescriptionsQuery->record().indexOf("id");
        const int creationDateIdx = d->selectDescriptionsQuery->record().indexOf("creation_date");
        const int aircraftTypeIdx = d->selectDescriptionsQuery->record().indexOf("type");
        const int startDateIdx = d->selectDescriptionsQuery->record().indexOf("start_date");
        const int startLocalSimulationTimeIdx = d->selectDescriptionsQuery->record().indexOf("start_local_sim_time");
        const int startZuluSimulationTimeIdx = d->selectDescriptionsQuery->record().indexOf("start_zulu_sim_time");
        const int startWaypointIdx = d->selectDescriptionsQuery->record().indexOf("start_waypoint");
        const int endDateIdx = d->selectDescriptionsQuery->record().indexOf("end_date");
        const int endLocalSimulationTimeIdx = d->selectDescriptionsQuery->record().indexOf("end_local_sim_time");
        const int endZuluSimulationTimeIdx = d->selectDescriptionsQuery->record().indexOf("end_zulu_sim_time");
        const int endWaypointIdx = d->selectDescriptionsQuery->record().indexOf("end_waypoint");
        const int titleIdx = d->selectDescriptionsQuery->record().indexOf("title");
        while (d->selectDescriptionsQuery->next()) {            

            FlightSummary description;
            description.id = d->selectDescriptionsQuery->value(idIdx).toLongLong();

            QDateTime dateTime = d->selectDescriptionsQuery->value(creationDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            description.creationDate = dateTime.toLocalTime();
            description.aircraftType = d->selectDescriptionsQuery->value(aircraftTypeIdx).toString();
            dateTime = d->selectDescriptionsQuery->value(startDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            description.startDate = dateTime.toLocalTime();
            // Persisted times is are already local respectively zulu simulation times
            description.startSimulationLocalTime = d->selectDescriptionsQuery->value(startLocalSimulationTimeIdx).toDateTime();
            description.startSimulationZuluTime = d->selectDescriptionsQuery->value(startZuluSimulationTimeIdx).toDateTime();
            description.startLocation = d->selectDescriptionsQuery->value(startWaypointIdx).toString();
            dateTime = d->selectDescriptionsQuery->value(endDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            description.endDate = dateTime.toLocalTime();
            // Persisted times is are already local respectively zulu simulation times
            description.endSimulationLocalTime = d->selectDescriptionsQuery->value(endLocalSimulationTimeIdx).toDateTime();
            description.endSimulationZuluTime = d->selectDescriptionsQuery->value(endZuluSimulationTimeIdx).toDateTime();
            description.endLocation = d->selectDescriptionsQuery->value(endWaypointIdx).toString();
            description.title = d->selectDescriptionsQuery->value(titleIdx).toString();

            summaries.append(description);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteFlightDao::getFlightDescriptions: SQL error: %s", qPrintable(d->selectDescriptionsQuery->lastError().databaseText() + " - error code: " + d->selectDescriptionsQuery->lastError().nativeErrorCode()));
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
