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
#include <utility>

#include <QString>
#include <QSqlDatabase>
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

struct SQLiteSecondaryFlightControlDaoPrivate
{
    SQLiteSecondaryFlightControlDaoPrivate(const QSqlDatabase &db) noexcept
        : db(db)
    {}

    QSqlDatabase db;
};

// PUBLIC

SQLiteSecondaryFlightControlDao::SQLiteSecondaryFlightControlDao(const QSqlDatabase &db) noexcept
    : d(std::make_unique<SQLiteSecondaryFlightControlDaoPrivate>(db))
{}

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
        "  left_leading_edge_flaps_position,"
        "  right_leading_edge_flaps_position,"
        "  left_trailing_edge_flaps_position,"
        "  right_trailing_edge_flaps_position,"
        "  left_spoilers_position,"
        "  right_spoilers_position,"
        "  spoilers_handle_percent,"
        "  spoilers_armed,"
        "  flaps_handle_index"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :left_leading_edge_flaps_position,"
        " :right_leading_edge_flaps_position,"
        " :left_trailing_edge_flaps_position,"
        " :right_trailing_edge_flaps_position,"
        " :left_spoilers_position,"
        " :right_spoilers_position,"
        " :spoilers_handle_percent,"
        " :spoilers_armed,"
        " :flaps_handle_index"
        ");"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(secondaryFlightControlData.timestamp));
    query.bindValue(":left_leading_edge_flaps_position", secondaryFlightControlData.leftLeadingEdgeFlapsPosition);
    query.bindValue(":right_leading_edge_flaps_position", secondaryFlightControlData.rightLeadingEdgeFlapsPosition);
    query.bindValue(":left_trailing_edge_flaps_position", secondaryFlightControlData.leftTrailingEdgeFlapsPosition);
    query.bindValue(":right_trailing_edge_flaps_position", secondaryFlightControlData.rightTrailingEdgeFlapsPosition);
    query.bindValue(":left_spoilers_position", secondaryFlightControlData.leftSpoilersPosition);
    query.bindValue(":right_spoilers_position", secondaryFlightControlData.rightSpoilersPosition);
    query.bindValue(":spoilers_handle_percent", secondaryFlightControlData.spoilersHandlePercent);
    query.bindValue(":spoilers_armed", secondaryFlightControlData.spoilersArmed);
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
        const bool querySizeFeature = d->db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            secondaryFlightControlData.reserve(query.size());
        } else {
            secondaryFlightControlData.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const int timestampIdx = record.indexOf("timestamp");
        const int leftLeadingEdgeFlapsPositionIdx = record.indexOf("left_leading_edge_flaps_position");
        const int rightLeadingEdgeFlapsPositionIdx = record.indexOf("right_leading_edge_flaps_position");
        const int leftTrailingEdgeFlapsPositionIdx = record.indexOf("left_trailing_edge_flaps_position");
        const int rightTrailingEdgeFlapsPositionIdx = record.indexOf("right_trailing_edge_flaps_position");
        const int leftSpoilersPositionIdx = record.indexOf("left_spoilers_position");
        const int rightSpoilersPositionIdx = record.indexOf("right_spoilers_position");
        const int spoilersHandlePercentIdx = record.indexOf("spoilers_handle_percent");
        const int spoilersArmedIdx = record.indexOf("spoilers_armed");
        const int flapsHandleIndexIdx = record.indexOf("flaps_handle_index");
        while (query.next()) {
            SecondaryFlightControlData data;
            data.timestamp = query.value(timestampIdx).toLongLong();
            data.leftLeadingEdgeFlapsPosition = static_cast<std::int16_t>(query.value(leftLeadingEdgeFlapsPositionIdx).toInt());
            data.rightLeadingEdgeFlapsPosition = static_cast<std::int16_t>(query.value(rightLeadingEdgeFlapsPositionIdx).toInt());
            data.leftTrailingEdgeFlapsPosition = static_cast<std::int16_t>(query.value(leftTrailingEdgeFlapsPositionIdx).toInt());
            data.rightTrailingEdgeFlapsPosition = static_cast<std::int16_t>(query.value(rightTrailingEdgeFlapsPositionIdx).toInt());
            data.leftSpoilersPosition = static_cast<std::int16_t>(query.value(leftSpoilersPositionIdx).toInt());
            data.rightSpoilersPosition = static_cast<std::int16_t>(query.value(rightSpoilersPositionIdx).toInt());
            data.spoilersHandlePercent = static_cast<std::uint8_t>(query.value(spoilersHandlePercentIdx).toInt());
            data.spoilersArmed = query.value(spoilersArmedIdx).toBool();
            data.flapsHandleIndex = static_cast<std::int8_t>(query.value(flapsHandleIndexIdx).toInt());

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
