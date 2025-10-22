/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#include <forward_list>
#include <utility>

#include <QString>
#include <QStringBuilder>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QDateTime>
#include <QTimeZone>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Model/Logbook.h>
#include <Model/FlightDate.h>
#include <Model/FlightSummary.h>
#include <Model/FlightCondition.h>
#include <FlightSelector.h>
#include "SQLiteLogbookDao.h"

namespace
{
    // The initial capacity of the logbook summaries vector (e.g. SQLite does not support returning
    // the result count for the given SELECT query)
    // Assume 50 entries per logbook
    constexpr int DefaultFlightCapacity = 50;
}

struct SQLiteLogbookDaoPrivate
{
    SQLiteLogbookDaoPrivate(QString connectionName) noexcept
        : connectionName {std::move(connectionName)}
    {}

    QString connectionName;
};

// PUBLIC

SQLiteLogbookDao::SQLiteLogbookDao(QString connectionName) noexcept
    : d {std::make_unique<SQLiteLogbookDaoPrivate>(std::move(connectionName))}
{}

SQLiteLogbookDao::SQLiteLogbookDao(SQLiteLogbookDao &&rhs) noexcept = default;
SQLiteLogbookDao &SQLiteLogbookDao::operator=(SQLiteLogbookDao &&rhs) noexcept = default;
SQLiteLogbookDao::~SQLiteLogbookDao() = default;

std::forward_list<FlightDate> SQLiteLogbookDao::getFlightDates(bool *ok) const noexcept
{
    std::forward_list<FlightDate> flightDates;
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select strftime('%Y', f.creation_time) as year, strftime('%m', f.creation_time) as month, "
        "strftime('%d', f.creation_time) as day, count(f.id) as nof_flights "
        "from  flight f "
        "group by year, month, day"
    );

    const bool success = query.exec();
    if (success) {
        QSqlRecord record = query.record();
        const auto yearIdx = record.indexOf("year");
        const auto monthIdx = record.indexOf("month");
        const auto dayIdx = record.indexOf("day");
        const auto nofFlightIdx = record.indexOf("nof_flights");
        while (query.next()) {
            const auto year = query.value(yearIdx).toInt();
            const auto month = query.value(monthIdx).toInt();
            const auto day = query.value(dayIdx).toInt();
            const auto nofFlights = query.value(nofFlightIdx).toInt();
            flightDates.emplace_front(year, month, day, nofFlights);
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteLogbookDao::getFlightDates: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return flightDates;
}

std::vector<FlightSummary> SQLiteLogbookDao::getFlightSummaries(const FlightSelector &flightSelector, bool *ok) const noexcept
{
    std::vector<FlightSummary> summaries;

    QString searchKeyword;
    if (!flightSelector.searchKeyword.isEmpty()) {
        const QString LikeOperatorPlaceholder {"%"};
        // Add like operator placeholders
        searchKeyword = LikeOperatorPlaceholder % flightSelector.searchKeyword % LikeOperatorPlaceholder;
    }

    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select f.id, f.creation_time, f.title, f.flight_number, a.type,"
        "       (select count(*) from aircraft where aircraft.flight_id = f.id) as aircraft_count,"
        "       f.start_local_sim_time, f.start_zulu_sim_time, fp1.ident as start_waypoint,"
        "       f.end_local_sim_time, f.end_zulu_sim_time, fp2.ident as end_waypoint "
        "from   flight f "
        "join   aircraft a "
        "on     a.flight_id = f.id "
        "and    a.seq_nr = f.user_aircraft_seq_nr "
        "join   aircraft_type at "
        "on     a.type = at.type "
        "left join (select ident, aircraft_id from waypoint wo1 where wo1.timestamp = (select min(wi1.timestamp) from waypoint wi1 where wi1.aircraft_id = wo1.aircraft_id)) fp1 "
        "on fp1.aircraft_id = a.id "
        "left join (select ident, aircraft_id from waypoint wo2 where wo2.timestamp = (select max(wi2.timestamp) from waypoint wi2 where wi2.aircraft_id = wo2.aircraft_id)) fp2 "
        "on fp2.aircraft_id = a.id "
        "where f.creation_time between :from_date and :to_date "
        "  and (   f.title like coalesce(:search_keyword, f.title) "
        "       or f.flight_number like coalesce(:search_keyword, f.flight_number) "
        "       or a.type like coalesce(:search_keyword, a.type) "
        "       or start_waypoint like coalesce(:search_keyword, start_waypoint) "
        "       or end_waypoint like coalesce(:search_keyword, end_waypoint) "
        "      ) "
        "  and aircraft_count > :aircraft_count "
        "  and at.engine_type = coalesce(:engine_type, at.engine_type)"
        "  and (   :duration = 0"
        "       or round((julianday(f.end_zulu_sim_time) - julianday(f.start_zulu_sim_time)) * 1440) >= :duration"
        "      );"
    );

    const auto aircraftCount = flightSelector.hasFormation ? 1 : 0;
    query.bindValue(":from_date", flightSelector.fromDate);
    query.bindValue(":to_date", flightSelector.toDate);
    query.bindValue(":search_keyword", searchKeyword);
    query.bindValue(":aircraft_count", aircraftCount);
    const QVariant engineTypeVariant = flightSelector.engineType != SimType::EngineType::All ? Enum::underly(flightSelector.engineType) : QVariant();
    query.bindValue(":engine_type", engineTypeVariant);
    query.bindValue(":duration", flightSelector.mininumDurationMinutes);
    const bool success = query.exec();
    if (success) {
        const auto db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            summaries.reserve(query.size());
        } else {
            summaries.reserve(::DefaultFlightCapacity);
        }
        QSqlRecord record = query.record();
        const auto idIdx = record.indexOf("id");
        const auto creationTimeIdx = record.indexOf("creation_time");
        const auto typeIdx = record.indexOf("type");
        const auto flightNumberIdx = record.indexOf("flight_number");
        const auto aircraftCountIdx = record.indexOf("aircraft_count");
        const auto startLocalSimulationTimeIdx = record.indexOf("start_local_sim_time");
        const auto startZuluSimulationTimeIdx = record.indexOf("start_zulu_sim_time");
        const auto startWaypointIdx = record.indexOf("start_waypoint");
        const auto endLocalSimulationTimeIdx = record.indexOf("end_local_sim_time");
        const auto endZuluSimulationTimeIdx = record.indexOf("end_zulu_sim_time");
        const auto endWaypointIdx = record.indexOf("end_waypoint");
        const auto titleIdx = record.indexOf("title");
        while (query.next()) {
            FlightSummary summary;
            summary.flightId = query.value(idIdx).toLongLong();

            QDateTime dateTime = query.value(creationTimeIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::UTC);
            summary.creationDate = dateTime.toLocalTime();
            summary.aircraftType = query.value(typeIdx).toString();
            summary.flightNumber = query.value(flightNumberIdx).toString();
            summary.aircraftCount = query.value(aircraftCountIdx).toInt();
            // Persisted times is are already local respectively zulu simulation times
            summary.startSimulationLocalTime = query.value(startLocalSimulationTimeIdx).toDateTime();
            summary.startSimulationZuluTime = query.value(startZuluSimulationTimeIdx).toDateTime();
            summary.startSimulationZuluTime.setTimeZone(QTimeZone::UTC);
            summary.startLocation = query.value(startWaypointIdx).toString();
            // Persisted times is are already local respectively zulu simulation times
            summary.endSimulationLocalTime = query.value(endLocalSimulationTimeIdx).toDateTime();
            summary.endSimulationZuluTime = query.value(endZuluSimulationTimeIdx).toDateTime();
            summary.endSimulationZuluTime.setTimeZone(QTimeZone::UTC);
            summary.endLocation = query.value(endWaypointIdx).toString();
            summary.title = query.value(titleIdx).toString();

            summaries.push_back(std::move(summary));
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteLogbookDao::getFlightSummaries: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return summaries;
}

std::vector<std::int64_t> SQLiteLogbookDao::getFlightIds(const FlightSelector &flightSelector, bool *ok) const noexcept
{
    std::vector<std::int64_t> flightIds;

    QString searchKeyword;
    if (!flightSelector.searchKeyword.isEmpty()) {
        const QString LikeOperatorPlaceholder {"%"};
        // Add like operator placeholders
        searchKeyword = LikeOperatorPlaceholder  % flightSelector.searchKeyword % LikeOperatorPlaceholder;
    }

    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select f.id,"
        "       (select count(*) from aircraft where aircraft.flight_id = f.id) as aircraft_count,"
        "       f.start_local_sim_time, f.start_zulu_sim_time, fp1.ident as start_waypoint,"
        "       f.end_local_sim_time, f.end_zulu_sim_time, fp2.ident as end_waypoint "
        "from   flight f "
        "join   aircraft a "
        "on     a.flight_id = f.id "
        "and    a.seq_nr = f.user_aircraft_seq_nr "
        "join   aircraft_type at "
        "on     a.type = at.type "
        "left join (select ident, aircraft_id from waypoint wo1 where wo1.timestamp = (select min(wi1.timestamp) from waypoint wi1 where wi1.aircraft_id = wo1.aircraft_id)) fp1 "
        "on fp1.aircraft_id = a.id "
        "left join (select ident, aircraft_id from waypoint wo2 where wo2.timestamp = (select max(wi2.timestamp) from waypoint wi2 where wi2.aircraft_id = wo2.aircraft_id)) fp2 "
        "on fp2.aircraft_id = a.id "
        "where f.creation_time between :from_date and :to_date "
        "  and (  f.title like coalesce(:search_keyword, f.title) "
        "       or a.type like coalesce(:search_keyword, a.type) "
        "       or start_waypoint like coalesce(:search_keyword, start_waypoint) "
        "       or end_waypoint like coalesce(:search_keyword, end_waypoint) "
        "      ) "
        "  and aircraft_count > :aircraft_count "
        "  and at.engine_type = coalesce(:engine_type, at.engine_type)"
        "  and (   :duration = 0"
        "       or round((julianday(f.end_zulu_sim_time) - julianday(f.start_zulu_sim_time)) * 1440) >= :duration"
        "      );"
    );

    const auto aircraftCount = flightSelector.hasFormation ? 1 : 0;
    query.bindValue(":from_date", flightSelector.fromDate);
    query.bindValue(":to_date", flightSelector.toDate);
    query.bindValue(":search_keyword", searchKeyword);
    query.bindValue(":aircraft_count", aircraftCount);
    const QVariant engineTypeVariant = flightSelector.engineType != SimType::EngineType::All ? Enum::underly(flightSelector.engineType) : QVariant();
    query.bindValue(":engine_type", engineTypeVariant);
    query.bindValue(":duration", flightSelector.mininumDurationMinutes);
    const bool success = query.exec();
    if (success) {
        const auto db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            flightIds.reserve(query.size());
        } else {
            flightIds.reserve(::DefaultFlightCapacity);
        }
        QSqlRecord record = query.record();
        const auto idIdx = record.indexOf("id");
        while (query.next()) {
            std::int64_t flightId = query.value(idIdx).toLongLong();
            flightIds.push_back(flightId);
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteLogbookDao::getFlightIds: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return flightIds;
}
