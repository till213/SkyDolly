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

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>

#include "../../../../Kernel/src/Enum.h"
#include "../../../../Model/src/SecondaryFlightControlData.h"
#include "../../ConnectionManager.h"
#include "SQLiteSecondaryFlightControlDao.h"

class SQLiteSecondaryFlightControlDaoPrivate
{
public:
    SQLiteSecondaryFlightControlDaoPrivate() noexcept
    {}

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByAircraftIdQuery;
    std::unique_ptr<QSqlQuery> deleteByFlightIdQuery;
    std::unique_ptr<QSqlQuery> deleteByIdQuery;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();

        }
        if (selectByAircraftIdQuery == nullptr) {
            selectByAircraftIdQuery = std::make_unique<QSqlQuery>();

        }
        if (deleteByFlightIdQuery == nullptr) {
            deleteByFlightIdQuery = std::make_unique<QSqlQuery>();

        }
        if (deleteByIdQuery == nullptr) {
            deleteByIdQuery = std::make_unique<QSqlQuery>();

        }
    }

    void resetQueries() noexcept
    {
        insertQuery = nullptr;
        selectByAircraftIdQuery = nullptr;
        deleteByFlightIdQuery = nullptr;
        deleteByIdQuery = nullptr;
    }
};

// PUBLIC

SQLiteSecondaryFlightControlDao::SQLiteSecondaryFlightControlDao() noexcept
{}

SQLiteSecondaryFlightControlDao::~SQLiteSecondaryFlightControlDao() noexcept
{}

bool SQLiteSecondaryFlightControlDao::add(qint64 aircraftId, const SecondaryFlightControlData &secondaryFlightControlData)  noexcept
{
    QSqlQuery query;
    query.prepare(
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
        ");"
    );

    query.bindValue(":aircraft_id", aircraftId);
    query.bindValue(":timestamp", secondaryFlightControlData.timestamp);
    query.bindValue(":leading_edge_flaps_left_percent", secondaryFlightControlData.leadingEdgeFlapsLeftPercent);
    query.bindValue(":leading_edge_flaps_right_percent", secondaryFlightControlData.leadingEdgeFlapsRightPercent);
    query.bindValue(":trailing_edge_flaps_left_percent", secondaryFlightControlData.trailingEdgeFlapsLeftPercent);
    query.bindValue(":trailing_edge_flaps_right_percent", secondaryFlightControlData.trailingEdgeFlapsRightPercent);
    query.bindValue(":spoilers_handle_position", secondaryFlightControlData.spoilersHandlePosition);
    query.bindValue(":flaps_handle_index", secondaryFlightControlData.flapsHandleIndex);

    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteSecondaryFlightControlDao::add: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteSecondaryFlightControlDao::getByAircraftId(qint64 aircraftId, std::insert_iterator<std::vector<SecondaryFlightControlData>> insertIterator) const noexcept
{
    QSqlQuery query;
    query.prepare(
        "select * "
        "from   secondary_flight_control sfc "
        "where  sfc.aircraft_id = :aircraft_id "
        "order by sfc.timestamp asc;"
    );

    query.bindValue(":aircraft_id", aircraftId);
    bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int timestampIdx = record.indexOf("timestamp");
        const int leadingEdgeFlapsLeftPercentIdx = record.indexOf("leading_edge_flaps_left_percent");
        const int leadingEdgeFlapsRightPercentIdx = record.indexOf("leading_edge_flaps_right_percent");
        const int trailingEdgeFlapsLeftPercentIdx = record.indexOf("trailing_edge_flaps_left_percent");
        const int trailingEdgeFlapsRightPercentIdx = record.indexOf("trailing_edge_flaps_right_percent");
        const int spoilersHandlePositionIdx = record.indexOf("spoilers_handle_position");
        const int flapsHandleIndexIdx = record.indexOf("flaps_handle_index");
        while (query.next()) {

            SecondaryFlightControlData data;

            data.timestamp = query.value(timestampIdx).toLongLong();
            data.leadingEdgeFlapsLeftPercent = query.value(leadingEdgeFlapsLeftPercentIdx).toInt();
            data.leadingEdgeFlapsRightPercent = query.value(leadingEdgeFlapsRightPercentIdx).toInt();
            data.trailingEdgeFlapsLeftPercent = query.value(trailingEdgeFlapsLeftPercentIdx).toInt();
            data.trailingEdgeFlapsRightPercent = query.value(trailingEdgeFlapsRightPercentIdx).toInt();
            data.spoilersHandlePosition = query.value(spoilersHandlePositionIdx).toInt();
            data.flapsHandleIndex = query.value(flapsHandleIndexIdx).toInt();

            insertIterator = std::move(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteSecondaryFlightControlDao::getByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }

    return ok;
}

bool SQLiteSecondaryFlightControlDao::deleteByFlightId(qint64 flightId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   secondary_flight_control "
        "where  aircraft_id in (select a.id "
        "                       from   aircraft a"
        "                       where  a.flight_id = :flight_id"
        "                      );"
    );

    query.bindValue(":flight_id", flightId);
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteSecondaryFlightControlDao::deleteByFlightId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteSecondaryFlightControlDao::deleteByAircraftId(qint64 aircraftId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   secondary_flight_control "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", aircraftId);
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteSecondaryFlightControlDao::deleteByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return true;
}
