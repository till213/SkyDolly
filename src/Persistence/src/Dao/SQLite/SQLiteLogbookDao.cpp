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
#include <forward_list>
#include <iterator>

#include <QString>
#include <QStringBuilder>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QTimeZone>

#include <Kernel/Enum.h>
#include <Model/Logbook.h>
#include <Model/FlightDate.h>
#include <Model/FlightSummary.h>
#include <Model/FlightCondition.h>
#include <FlightSelector.h>
#include "SQLiteLogbookDao.h"

// PUBLIC

SQLiteLogbookDao::SQLiteLogbookDao() noexcept
{}

SQLiteLogbookDao::~SQLiteLogbookDao() noexcept
{}

bool SQLiteLogbookDao::getFlightDates(std::front_insert_iterator<std::forward_list<FlightDate>> frontInsertIterator) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select strftime('%Y', f.creation_time) as year, strftime('%m', f.creation_time) as month, strftime('%d', f.creation_time) as day, count(f.id) as nof_flights "
        "from  flight f "
        "group by year, month, day"
    );

    const bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int yearIdx = record.indexOf("year");
        const int monthIdx = record.indexOf("month");
        const int dayIdx = record.indexOf("day");
        const int nofFlightIdx = record.indexOf("nof_flights");
        while (query.next()) {
            const int year = query.value(yearIdx).toInt();
            const int month = query.value(monthIdx).toInt();
            const int day = query.value(dayIdx).toInt();
            const int nofFlights = query.value(nofFlightIdx).toInt();
            frontInsertIterator = std::move(FlightDate(year, month, day, nofFlights));
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteLogbookDao::getFlightDates: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }

    return ok;
}

std::vector<FlightSummary> SQLiteLogbookDao::getFlightSummaries(const FlightSelector &flightSelector) const noexcept
{
    const QString LikeOperatorPlaceholder("%");

    std::vector<FlightSummary> summaries;
    QString searchKeyword;
    if (!flightSelector.searchKeyword.isEmpty()) {
        // Add like operator placeholders
        searchKeyword = LikeOperatorPlaceholder  % flightSelector.searchKeyword % LikeOperatorPlaceholder;
    }

    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select f.id, f.creation_time, f.title, a.type,"
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

    const int aircraftCount = flightSelector.hasFormation ? 1 : 0;

    query.bindValue(":from_date", flightSelector.fromDate);
    query.bindValue(":to_date", flightSelector.toDate);
    query.bindValue(":search_keyword", searchKeyword);
    query.bindValue(":aircraft_count", aircraftCount);
    const QVariant engineTypeVariant = flightSelector.engineType != SimType::EngineType::All ? Enum::toUnderlyingType(flightSelector.engineType) : QVariant();
    query.bindValue(":engine_type", engineTypeVariant);
    query.bindValue(":duration", flightSelector.mininumDurationMinutes);
    const bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int idIdx = record.indexOf("id");
        const int creationTimeIdx = record.indexOf("creation_time");
        const int typeIdx = record.indexOf("type");
        const int aircraftCountIdx = record.indexOf("aircraft_count");
        const int startLocalSimulationTimeIdx = record.indexOf("start_local_sim_time");
        const int startZuluSimulationTimeIdx = record.indexOf("start_zulu_sim_time");
        const int startWaypointIdx = record.indexOf("start_waypoint");
        const int endLocalSimulationTimeIdx = record.indexOf("end_local_sim_time");
        const int endZuluSimulationTimeIdx = record.indexOf("end_zulu_sim_time");
        const int endWaypointIdx = record.indexOf("end_waypoint");
        const int titleIdx = record.indexOf("title");
        while (query.next()) {

            FlightSummary summary;
            summary.flightId = query.value(idIdx).toLongLong();

            QDateTime dateTime = query.value(creationTimeIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            summary.creationDate = dateTime.toLocalTime();
            summary.aircraftType = query.value(typeIdx).toString();
            summary.aircraftCount = query.value(aircraftCountIdx).toInt();
            // Persisted times is are already local respectively zulu simulation times
            summary.startSimulationLocalTime = query.value(startLocalSimulationTimeIdx).toDateTime();
            summary.startSimulationZuluTime = query.value(startZuluSimulationTimeIdx).toDateTime();
            summary.startLocation = query.value(startWaypointIdx).toString();
            // Persisted times is are already local respectively zulu simulation times
            summary.endSimulationLocalTime = query.value(endLocalSimulationTimeIdx).toDateTime();
            summary.endSimulationZuluTime = query.value(endZuluSimulationTimeIdx).toDateTime();
            summary.endLocation = query.value(endWaypointIdx).toString();
            summary.title = query.value(titleIdx).toString();

            summaries.push_back(summary);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteLogbookDao::getFlightSummaries: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }

    return summaries;
}
