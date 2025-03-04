/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <utility>

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

#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include "SQLiteWaypointDao.h"

struct SQLiteWaypointDaoPrivate
{
    SQLiteWaypointDaoPrivate(QString connectionName) noexcept
        : connectionName {std::move(connectionName)}
    {}

    QString connectionName;
};

// PUBLIC

SQLiteWaypointDao::SQLiteWaypointDao(QString connectionName) noexcept
    : d {std::make_unique<SQLiteWaypointDaoPrivate>(std::move(connectionName))}
{}

SQLiteWaypointDao::SQLiteWaypointDao(SQLiteWaypointDao &&rhs) noexcept = default;
SQLiteWaypointDao &SQLiteWaypointDao::operator=(SQLiteWaypointDao &&rhs) noexcept = default;
SQLiteWaypointDao::~SQLiteWaypointDao() = default;

bool SQLiteWaypointDao::add(std::int64_t aircraftId, const FlightPlan &flightPlan) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "insert into waypoint ("
        "  aircraft_id,"
        "  timestamp,"
        "  ident,"
        "  latitude,"
        "  longitude,"
        "  altitude,"
        "  local_sim_time,"
        "  zulu_sim_time"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :ident,"
        " :latitude,"
        " :longitude,"
        " :altitude,"
        " :local_sim_time,"
        " :zulu_sim_time"
        ");"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    bool ok {true};
    for (const auto &waypoint : flightPlan) {
        query.bindValue(":timestamp", QVariant::fromValue(waypoint.timestamp));
        query.bindValue(":ident", waypoint.identifier);
        query.bindValue(":latitude", waypoint.latitude);
        query.bindValue(":longitude", waypoint.longitude);
        query.bindValue(":altitude", waypoint.altitude);
        // No conversion to UTC
        query.bindValue(":local_sim_time", waypoint.localTime);
        // Zulu time equals to UTC time
        query.bindValue(":zulu_sim_time", waypoint.zuluTime);

        ok = query.exec();
        if (!ok) {
#ifdef DEBUG
            qDebug() << "SQLiteWaypointDao::add: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
            break;
        }
    }
    return ok;
}

bool SQLiteWaypointDao::getByAircraftId(std::int64_t aircraftId, FlightPlan &flightPlan) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   waypoint w "
        "where  w.aircraft_id = :aircraft_id "
        "order by w.timestamp asc;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
    if (ok) {
        flightPlan.clear();
        QSqlRecord record = query.record();
        const auto timestampIdx = record.indexOf("timestamp");
        const auto identifierIdx = record.indexOf("ident");
        const auto latitudeIdx = record.indexOf("latitude");
        const auto longitudeIdx = record.indexOf("longitude");
        const auto altitudeIdx = record.indexOf("altitude");
        const auto localSimulationTimeIdx = record.indexOf("local_sim_time");
        const auto zuluSimulationTimeIdx = record.indexOf("zulu_sim_time");
        while (query.next()) {
            Waypoint data;
            data.timestamp = query.value(timestampIdx).toLongLong();
            data.identifier = query.value(identifierIdx).toString();
            data.latitude = query.value(latitudeIdx).toFloat();
            data.longitude = query.value(longitudeIdx).toFloat();
            data.altitude = query.value(altitudeIdx).toFloat();
            // Persisted time is already local simulation time
            data.localTime = query.value(localSimulationTimeIdx).toDateTime();
            // UTC equals zulu time, so no conversion necessary
            data.zuluTime = query.value(zuluSimulationTimeIdx).toDateTime();
            data.zuluTime.setTimeZone(QTimeZone::UTC);
            flightPlan.add(std::move(data));
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteWaypointDao::getByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    return ok;
}

bool SQLiteWaypointDao::deleteByFlightId(std::int64_t flightId) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   waypoint "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteWaypointDao::deleteByFlightId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteWaypointDao::deleteByAircraftId(std::int64_t aircraftId) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   waypoint "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteWaypointDao::deleteByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return true;
}
