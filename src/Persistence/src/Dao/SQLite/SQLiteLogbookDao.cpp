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
#include <iterator>

#include <QObject>
#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QTimeZone>

#include "../../../../Kernel/src/Enum.h"
#include "../../../../Model/src/Logbook.h"
#include "../../../../Model/src/FlightSummary.h"
#include "../../../../Model/src/FlightCondition.h"
#include "../../Dao/AircraftDaoIntf.h"
#include "../../Dao/DaoFactory.h"
#include "../../ConnectionManager.h"
#include "SQLiteLogbookDao.h"
#include "SQLiteLogbookDao.h"

class SQLiteLogbookDaoPrivate
{
public:
    SQLiteLogbookDaoPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          aircraftDao(daoFactory->createAircraftDao())
    {}

    std::unique_ptr<QSqlQuery> selectSummariesQuery;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftDaoIntf> aircraftDao;

    void initQueries()
    {
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

QVector<FlightSummary> SQLiteLogbookDao::getFlightSummaries() const noexcept
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
