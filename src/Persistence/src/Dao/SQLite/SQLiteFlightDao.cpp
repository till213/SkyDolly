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
#include "../../../../Model/src/FlightDescription.h"
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
"  local_sim_time,"
"  zulu_sim_time"
") values ("
"  null,"
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
" :local_sim_time,"
" :zulu_sim_time"
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
        if (selectDescriptionsQuery == nullptr) {
            selectDescriptionsQuery = std::make_unique<QSqlQuery>();
            selectDescriptionsQuery->setForwardOnly(true);
            selectDescriptionsQuery->prepare(
"select f.id, f.creation_date, f.description, a.type, a.start_date, fp1.ident as start_waypoint, a.end_date, fp2.ident as end_waypoint "
"from   flight f "
"join   aircraft a "
"on     a.flight_id = f.id "
"left join (select ident, aircraft_id from flight_plan fpo1 where fpo1.timestamp = (select min(fpi1.timestamp) from flight_plan fpi1 where fpi1.aircraft_id = fpo1.aircraft_id)) fp1 "
"on fp1.aircraft_id = a.id "
"left join (select ident, aircraft_id from flight_plan fpo2 where fpo2.timestamp = (select max(fpi2.timestamp) from flight_plan fpi2 where fpi2.aircraft_id = fpo2.aircraft_id)) fp2 "
"on fp2.aircraft_id = a.id");
        }
    }

    void resetQueries() noexcept
    {
        insertQuery = nullptr;
        selectByIdQuery = nullptr;
        deleteByIdQuery = nullptr;
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
    d->insertQuery->bindValue(":local_sim_time", flightCondition.localTime);
    // Zulu time equals to UTC time
    d->insertQuery->bindValue(":zulu_sim_time", flightCondition.zuluTime);
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
        ok = d->aircraftDao->add(flight.getId(), UserAircraftSequenceNumber, flight.getUserAircraft());
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
        int idIdx = d->selectByIdQuery->record().indexOf("id");
        int creationDateIdx = d->selectByIdQuery->record().indexOf("creation_date");
        int descriptionIdx = d->selectByIdQuery->record().indexOf("description");
        int surfaceTypeIdx = d->selectByIdQuery->record().indexOf("surface_type");
        int groundAltitudeIdx = d->selectByIdQuery->record().indexOf("ground_altitude");
        int ambientTemperatureIdx = d->selectByIdQuery->record().indexOf("ambient_temperature");
        int totalAirTemperatureIdx = d->selectByIdQuery->record().indexOf("total_air_temperature");
        int windVelocityIdx = d->selectByIdQuery->record().indexOf("wind_velocity");
        int windDirectionIdx = d->selectByIdQuery->record().indexOf("wind_direction");
        int visibilityIdx = d->selectByIdQuery->record().indexOf("visibility");
        int seaLevelPressureIdx = d->selectByIdQuery->record().indexOf("sea_level_pressure");
        int pitotIcingIdx = d->selectByIdQuery->record().indexOf("pitot_icing");
        int structuralIcingIdx = d->selectByIdQuery->record().indexOf("structural_icing");
        int precipitationStateIdx = d->selectByIdQuery->record().indexOf("precipitation_state");
        int inCloudsIdx = d->selectByIdQuery->record().indexOf("in_clouds");
        int localSimulationTimeIdx = d->selectByIdQuery->record().indexOf("local_sim_time");
        int zuluSimulationTimeIdx = d->selectByIdQuery->record().indexOf("zulu_sim_time");
        if (d->selectByIdQuery->next()) {
            flight.setId(d->selectByIdQuery->value(idIdx).toLongLong());
            QDateTime date = d->selectByIdQuery->value(creationDateIdx).toDateTime();
            date.setTimeZone(QTimeZone::utc());
            flight.setCreationDate(date.toLocalTime());
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
            // Persisted time is already local simulation time
            flightCondition.localTime = d->selectByIdQuery->value(localSimulationTimeIdx).toDateTime();
            // UTC equals zulu time, so no conversion necessary
            flightCondition.zuluTime = d->selectByIdQuery->value(zuluSimulationTimeIdx).toDateTime();

            flight.setFlightCondition(flightCondition);
        }
        Aircraft &userAircraft = flight.getUserAircraft();
        ok = d->aircraftDao->getByFlightId(id, UserAircraftSequenceNumber, userAircraft);
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

QVector<FlightDescription> SQLiteFlightDao::getFlightDescriptions() const noexcept
{
    QVector<FlightDescription> descriptions;

    d->initQueries();
    bool ok = d->selectDescriptionsQuery->exec();
    if (ok) {
        int idIdx = d->selectDescriptionsQuery->record().indexOf("id");
        int creationDateIdx = d->selectDescriptionsQuery->record().indexOf("creation_date");
        int aircraftTypeIdx = d->selectDescriptionsQuery->record().indexOf("type");
        int startDateIdx = d->selectDescriptionsQuery->record().indexOf("start_date");
        int startWaypointIdx = d->selectDescriptionsQuery->record().indexOf("start_waypoint");
        int endDateIdx = d->selectDescriptionsQuery->record().indexOf("end_date");
        int endWaypointIdx = d->selectDescriptionsQuery->record().indexOf("end_waypoint");
        int descriptionIdx = d->selectDescriptionsQuery->record().indexOf("description");
        while (d->selectDescriptionsQuery->next()) {            

            FlightDescription description;
            description.id = d->selectDescriptionsQuery->value(idIdx).toLongLong();

            QDateTime dateTime = d->selectDescriptionsQuery->value(creationDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            description.creationDate = dateTime.toLocalTime();
            description.aircraftType = d->selectDescriptionsQuery->value(aircraftTypeIdx).toString();
            dateTime = d->selectDescriptionsQuery->value(startDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            description.startDate = dateTime.toLocalTime();
            description.startLocation = d->selectDescriptionsQuery->value(startWaypointIdx).toString();
            dateTime = d->selectDescriptionsQuery->value(endDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            description.endDate = dateTime.toLocalTime();
            description.endLocation = d->selectDescriptionsQuery->value(endWaypointIdx).toString();
            description.description = d->selectDescriptionsQuery->value(descriptionIdx).toString();

            descriptions.append(description);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteFlightDao::getFlightDescriptions: SQL error: %s", qPrintable(d->selectDescriptionsQuery->lastError().databaseText() + " - error code: " + d->selectDescriptionsQuery->lastError().nativeErrorCode()));
#endif
    }

    return descriptions;
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
