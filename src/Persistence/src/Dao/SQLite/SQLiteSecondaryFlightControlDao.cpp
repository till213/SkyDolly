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

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>

#include "../../../../Kernel/src/Enum.h"
#include "../../../../Model/src/SecondaryFlightControlData.h"
#include "SQLiteSecondaryFlightControlDao.h"

class SQLiteSecondaryFlightControlDaoPrivate
{
public:
    SQLiteSecondaryFlightControlDaoPrivate() noexcept
        : insertQuery(nullptr),
          selectByAircraftIdQuery(nullptr)
    {
    }

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByAircraftIdQuery;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();
            insertQuery->prepare(
"insert into secondary_flight_control ("
"  aircraft_id,"
"  timestamp,"
"  leading_edge_flaps_left_percent,"
"  leading_edge_flaps_right_percent,"
"  trailing_edge_flaps_left_percent,"
"  trailing_edge_flaps_right_percent,"
"  spoilers_handle_position,"
"  flaps_handle_index"
") values ("
" :aircraft_id,"
" :timestamp,"
" :leading_edge_flaps_left_percent,"
" :leading_edge_flaps_right_percent,"
" :trailing_edge_flaps_left_percent,"
" :trailing_edge_flaps_right_percent,"
" :spoilers_handle_position,"
" :flaps_handle_index"
");");
        }
        if (selectByAircraftIdQuery == nullptr) {
            selectByAircraftIdQuery = std::make_unique<QSqlQuery>();
            selectByAircraftIdQuery->prepare(
"select * "
"from   secondary_flight_control sfc "
"where  sfc.aircraft_id = :aircraft_id "
"order by sfc.timestamp asc;");
        }
    }
};

// PUBLIC

SQLiteSecondaryFlightControlDao::SQLiteSecondaryFlightControlDao() noexcept
    : d(std::make_unique<SQLiteSecondaryFlightControlDaoPrivate>())
{
}

SQLiteSecondaryFlightControlDao::~SQLiteSecondaryFlightControlDao() noexcept
{}

bool SQLiteSecondaryFlightControlDao::add(qint64 aircraftId, const SecondaryFlightControlData &secondaryFlightControlData)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId, QSql::In);
    d->insertQuery->bindValue(":timestamp", secondaryFlightControlData.timestamp, QSql::In);
    d->insertQuery->bindValue(":leading_edge_flaps_left_percent", secondaryFlightControlData.leadingEdgeFlapsLeftPercent, QSql::In);
    d->insertQuery->bindValue(":leading_edge_flaps_right_percent", secondaryFlightControlData.leadingEdgeFlapsRightPercent, QSql::In);
    d->insertQuery->bindValue(":trailing_edge_flaps_left_percent", secondaryFlightControlData.trailingEdgeFlapsLeftPercent, QSql::In);
    d->insertQuery->bindValue(":trailing_edge_flaps_right_percent", secondaryFlightControlData.trailingEdgeFlapsRightPercent, QSql::In);
    d->insertQuery->bindValue(":spoilers_handle_position", secondaryFlightControlData.spoilersHandlePosition, QSql::In);
    d->insertQuery->bindValue(":flaps_handle_index", secondaryFlightControlData.flapsHandleIndex, QSql::In);

    bool ok = d->insertQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteSecondaryFlightControlDao::add: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteSecondaryFlightControlDao::getByAircraftId(qint64 aircraftId, QVector<SecondaryFlightControlData> &secondaryFlightControlData) const noexcept
{
    d->initQueries();
    d->selectByAircraftIdQuery->bindValue(":aircraft_id", aircraftId);
    bool ok = d->selectByAircraftIdQuery->exec();
    if (ok) {
        secondaryFlightControlData.clear();
        int timestampIdx = d->selectByAircraftIdQuery->record().indexOf("timestamp");
        int leadingEdgeFlapsLeftPercentIdx = d->selectByAircraftIdQuery->record().indexOf("leading_edge_flaps_left_percent");
        int leadingEdgeFlapsRightPercentIdx = d->selectByAircraftIdQuery->record().indexOf("leading_edge_flaps_right_percent");
        int trailingEdgeFlapsLeftPercentIdx = d->selectByAircraftIdQuery->record().indexOf("trailing_edge_flaps_left_percent");
        int trailingEdgeFlapsRightPercentIdx = d->selectByAircraftIdQuery->record().indexOf("trailing_edge_flaps_right_percent");
        int spoilersHandlePositionIdx = d->selectByAircraftIdQuery->record().indexOf("spoilers_handle_position");
        int flapsHandleIndexIdx = d->selectByAircraftIdQuery->record().indexOf("flaps_handle_index");
        while (d->selectByAircraftIdQuery->next()) {

            SecondaryFlightControlData data;

            data.timestamp = d->selectByAircraftIdQuery->value(timestampIdx).toLongLong();
            data.leadingEdgeFlapsLeftPercent = d->selectByAircraftIdQuery->value(leadingEdgeFlapsLeftPercentIdx).toInt();
            data.leadingEdgeFlapsRightPercent = d->selectByAircraftIdQuery->value(leadingEdgeFlapsRightPercentIdx).toInt();
            data.trailingEdgeFlapsLeftPercent = d->selectByAircraftIdQuery->value(trailingEdgeFlapsLeftPercentIdx).toInt();
            data.trailingEdgeFlapsRightPercent = d->selectByAircraftIdQuery->value(trailingEdgeFlapsRightPercentIdx).toInt();
            data.spoilersHandlePosition = d->selectByAircraftIdQuery->value(spoilersHandlePositionIdx).toInt();
            data.flapsHandleIndex = d->selectByAircraftIdQuery->value(flapsHandleIndexIdx).toInt();

            secondaryFlightControlData.append(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteSecondaryFlightControlDao::getByAircraftId: SQL error: %s", qPrintable(d->selectByAircraftIdQuery->lastError().databaseText() + " - error code: " + d->selectByAircraftIdQuery->lastError().nativeErrorCode()));
#endif
    }

    return ok;
}
