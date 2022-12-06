/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#include <cstdint>

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDriver>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Model/SecondaryFlightControlData.h>
#include "SQLiteSecondaryFlightControlDao.h"

namespace
{
    // The initial capacity of the secondary flight control vector (e.g. SQLite does not support returning
    // the result count for the given SELECT query)
    // Samples at 30 Hz for an assumed flight duration of 2 * 60 seconds = 2 minutes
    constexpr int DefaultCapacity = 30 * 2 * 60;
}

// PUBLIC

SQLiteSecondaryFlightControlDao::SQLiteSecondaryFlightControlDao(SQLiteSecondaryFlightControlDao &&rhs) noexcept = default;
SQLiteSecondaryFlightControlDao &SQLiteSecondaryFlightControlDao::operator=(SQLiteSecondaryFlightControlDao &&rhs) noexcept = default;
SQLiteSecondaryFlightControlDao::~SQLiteSecondaryFlightControlDao() = default;

bool SQLiteSecondaryFlightControlDao::add(std::int64_t aircraftId, const SecondaryFlightControlData &secondaryFlightControlData) noexcept
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

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(secondaryFlightControlData.timestamp));
    query.bindValue(":leading_edge_flaps_left_percent", secondaryFlightControlData.leadingEdgeFlapsLeftPosition);
    query.bindValue(":leading_edge_flaps_right_percent", secondaryFlightControlData.leadingEdgeFlapsRightPosition);
    query.bindValue(":trailing_edge_flaps_left_percent", secondaryFlightControlData.trailingEdgeFlapsLeftPosition);
    query.bindValue(":trailing_edge_flaps_right_percent", secondaryFlightControlData.trailingEdgeFlapsRightPosition);
    query.bindValue(":spoilers_handle_position", secondaryFlightControlData.spoilersHandlePosition);
    query.bindValue(":flaps_handle_index", secondaryFlightControlData.flapsHandleIndex);

    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteSecondaryFlightControlDao::add: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

std::vector<SecondaryFlightControlData> SQLiteSecondaryFlightControlDao::getByAircraftId(std::int64_t aircraftId, bool *ok) const noexcept
{
    std::vector<SecondaryFlightControlData> secondaryFlightControlData;
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   secondary_flight_control sfc "
        "where  sfc.aircraft_id = :aircraft_id "
        "order by sfc.timestamp asc;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool success = query.exec();
    if (success) {
        const bool querySizeFeature = QSqlDatabase::database().driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            secondaryFlightControlData.reserve(query.size());
        } else {
            secondaryFlightControlData.reserve(::DefaultCapacity);
        }
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
            data.leadingEdgeFlapsLeftPosition = query.value(leadingEdgeFlapsLeftPercentIdx).toInt();
            data.leadingEdgeFlapsRightPosition = query.value(leadingEdgeFlapsRightPercentIdx).toInt();
            data.trailingEdgeFlapsLeftPosition = query.value(trailingEdgeFlapsLeftPercentIdx).toInt();
            data.trailingEdgeFlapsRightPosition = query.value(trailingEdgeFlapsRightPercentIdx).toInt();
            data.spoilersHandlePosition = query.value(spoilersHandlePositionIdx).toInt();
            data.flapsHandleIndex = query.value(flapsHandleIndexIdx).toInt();

            secondaryFlightControlData.push_back(std::move(data));
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteSecondaryFlightControlDao::getByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return secondaryFlightControlData;
}

bool SQLiteSecondaryFlightControlDao::deleteByFlightId(std::int64_t flightId) noexcept
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

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteSecondaryFlightControlDao::deleteByFlightId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteSecondaryFlightControlDao::deleteByAircraftId(std::int64_t aircraftId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   secondary_flight_control "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteSecondaryFlightControlDao::deleteByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return true;
}
