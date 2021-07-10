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
#include <vector>
#include <forward_list>
#include <iterator>

#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QTimeZone>

#include "../../../../Kernel/src/Enum.h"
#include "../../../../Model/src/Logbook.h"
#include "../../../../Model/src/FlightDate.h"
#include "../../../../Model/src/FlightSummary.h"
#include "../../../../Model/src/FlightCondition.h"
#include "../../Dao/FlightSelector.h"
#include "../../Dao/DaoFactory.h"
#include "../../ConnectionManager.h"
#include "SQLiteLogbookDao.h"
#include "SQLiteLogbookDao.h"

class SQLiteLogbookDaoPrivate
{
public:
    SQLiteLogbookDaoPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite))
    {}

    std::unique_ptr<QSqlQuery> selectFlightDatesQuery;
    std::unique_ptr<QSqlQuery> selectSummariesQuery;
    std::unique_ptr<DaoFactory> daoFactory;

    void initQueries()
    {
        if (selectFlightDatesQuery == nullptr) {
            selectFlightDatesQuery = std::make_unique<QSqlQuery>();
            selectFlightDatesQuery->setForwardOnly(true);
            selectFlightDatesQuery->prepare(
"select strftime('%Y', creation_date) as year, strftime('%m', creation_date) as month, strftime('%d', creation_date) as day, count(flight.id) as nof_flights "
"from  flight "
"group by year, month, day");
        }
        if (selectSummariesQuery == nullptr) {
            selectSummariesQuery = std::make_unique<QSqlQuery>();
            selectSummariesQuery->setForwardOnly(true);
            selectSummariesQuery->prepare(
"select f.id, f.creation_date, f.title, at.title as aircraft_type, (select count(*) from aircraft where aircraft.flight_id = f.id) as aircraft_count,"
"       a.start_date, f.start_local_sim_time, f.start_zulu_sim_time, fp1.ident as start_waypoint,"
"       a.end_date, f.end_local_sim_time, f.end_zulu_sim_time, fp2.ident as end_waypoint "
"from   flight f "
"join   aircraft a "
"on     a.flight_id = f.id "
"and    a.seq_nr = 1 "
"join   aircraft_type at "
"on     a.aircraft_type_id = at.id "
"left join (select ident, aircraft_id from waypoint wo1 where wo1.timestamp = (select min(wi1.timestamp) from waypoint wi1 where wi1.aircraft_id = wo1.aircraft_id)) fp1 "
"on fp1.aircraft_id = a.id "
"left join (select ident, aircraft_id from waypoint wo2 where wo2.timestamp = (select max(wi2.timestamp) from waypoint wi2 where wi2.aircraft_id = wo2.aircraft_id)) fp2 "
"on fp2.aircraft_id = a.id "
"where f.creation_date between :from_date and :to_date "
"and   (  f.title like coalesce(:search_keyword, f.title) "
"       or at.title like coalesce(:search_keyword, at.title) "
"       or start_waypoint like coalesce(:search_keyword, start_waypoint) "
"       or end_waypoint like coalesce(:search_keyword, end_waypoint) "
"      )");
        }
    }

    void resetQueries() noexcept
    {
        selectFlightDatesQuery = nullptr;
        selectSummariesQuery = nullptr;
    }
};

// PUBLIC

SQLiteLogbookDao::SQLiteLogbookDao(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SQLiteLogbookDaoPrivate>())
{
    frenchConnection();
}

SQLiteLogbookDao::~SQLiteLogbookDao() noexcept
{}

std::forward_list<FlightDate> SQLiteLogbookDao::getFlightDates() const noexcept
{
    std::forward_list<FlightDate> flightDates;

    d->initQueries();
    const bool ok = d->selectFlightDatesQuery->exec();
    if (ok) {
        QSqlRecord record = d->selectFlightDatesQuery->record();
        const int yearIdx = record.indexOf("year");
        const int monthIdx = record.indexOf("month");
        const int dayIdx = record.indexOf("day");
        const int nofFlightIdx = record.indexOf("nof_flights");
        while (d->selectFlightDatesQuery->next()) {
            const int year = d->selectFlightDatesQuery->value(yearIdx).toInt();
            const int month = d->selectFlightDatesQuery->value(monthIdx).toInt();
            const int day = d->selectFlightDatesQuery->value(dayIdx).toInt();
            const int nofFlights = d->selectFlightDatesQuery->value(nofFlightIdx).toInt();
            flightDates.emplace_front(year, month, day, nofFlights);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteLogbookDao::getFlightDates: SQL error: %s", qPrintable(d->selectFlightDatesQuery->lastError().databaseText() + " - error code: " + d->selectFlightDatesQuery->lastError().nativeErrorCode()));
#endif
    }

    return flightDates;
}

std::vector<FlightSummary> SQLiteLogbookDao::getFlightSummaries(const FlightSelector &flightSelector) const noexcept
{
    const QString LikeOperatorPlaceholder("%");

    std::vector<FlightSummary> summaries;
    QString searchKeyword;
    if (!flightSelector.searchKeyword.isEmpty()) {
        // Case-insensitive search, add like operator placeholders
        searchKeyword = LikeOperatorPlaceholder  % flightSelector.searchKeyword.toLower() % LikeOperatorPlaceholder;
    }

    d->initQueries();
    d->selectSummariesQuery->bindValue(":from_date", flightSelector.fromDate);
    d->selectSummariesQuery->bindValue(":to_date", flightSelector.toDate);
    d->selectSummariesQuery->bindValue(":search_keyword", searchKeyword);
    const bool ok = d->selectSummariesQuery->exec();
    if (ok) {
        QSqlRecord record = d->selectSummariesQuery->record();
        const int idIdx = record.indexOf("id");
        const int creationDateIdx = record.indexOf("creation_date");
        const int aircraftTypeIdx = record.indexOf("aircraft_type");
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

            summaries.push_back(summary);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteLogbookDao::getFlightSummaries: SQL error: %s", qPrintable(d->selectSummariesQuery->lastError().databaseText() + " - error code: " + d->selectSummariesQuery->lastError().nativeErrorCode()));
#endif
    }

    return summaries;
}

// PRIVATE

void SQLiteLogbookDao::frenchConnection() noexcept
{
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &SQLiteLogbookDao::handleConnectionChanged);
}

// PRIVATE SLOTS

void SQLiteLogbookDao::handleConnectionChanged() noexcept
{
    d->resetQueries();
}
