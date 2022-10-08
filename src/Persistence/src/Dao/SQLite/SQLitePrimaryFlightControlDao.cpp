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
#include <Model/PrimaryFlightControlData.h>
#include "SQLitePrimaryFlightControlDao.h"

namespace
{
    // The initial capacity of the primary flight control vector (e.g. SQLite does not support returning
    // the result count for the given SELECT query)
    // Samples at 30 Hz for an assumed flight duration of 2 * 60 seconds = 2 minutes
    constexpr int DefaultCapacity = 30 * 2 * 60;
}

// PUBLIC

SQLitePrimaryFlightControlDao::SQLitePrimaryFlightControlDao(SQLitePrimaryFlightControlDao &&rhs) = default;
SQLitePrimaryFlightControlDao &SQLitePrimaryFlightControlDao::operator=(SQLitePrimaryFlightControlDao &&rhs) = default;
SQLitePrimaryFlightControlDao::~SQLitePrimaryFlightControlDao() = default;

bool SQLitePrimaryFlightControlDao::add(std::int64_t aircraftId, const PrimaryFlightControlData &primaryFlightControlData)  noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into primary_flight_control ("
        "  aircraft_id,"
        "  timestamp,"
        "  rudder_position,"
        "  elevator_position,"
        "  aileron_position"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :rudder_position,"
        " :elevator_position,"
        " :aileron_position"
        ");"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(primaryFlightControlData.timestamp));
    query.bindValue(":rudder_position", primaryFlightControlData.rudderPosition);
    query.bindValue(":elevator_position", primaryFlightControlData.elevatorPosition);
    query.bindValue(":aileron_position", primaryFlightControlData.aileronPosition);

    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLitePrimaryFlightControlDao::add: SQL error" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

std::vector<PrimaryFlightControlData> SQLitePrimaryFlightControlDao::getByAircraftId(std::int64_t aircraftId, bool *ok) const noexcept
{
    std::vector<PrimaryFlightControlData> primaryFlightControlData;
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   primary_flight_control pfc "
        "where  pfc.aircraft_id = :aircraft_id "
        "order by pfc.timestamp asc;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool success = query.exec();
    if (success) {
        const bool querySizeFeature = QSqlDatabase::database().driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            primaryFlightControlData.reserve(query.size());
        } else {
            primaryFlightControlData.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const int timestampIdx = record.indexOf("timestamp");
        const int rudderPositionIdx = record.indexOf("rudder_position");
        const int elevatorPositionIdx = record.indexOf("elevator_position");
        const int aileronPositionIdx = record.indexOf("aileron_position");
        while (query.next()) {

            PrimaryFlightControlData data;

            data.timestamp = query.value(timestampIdx).toLongLong();
            data.rudderPosition = query.value(rudderPositionIdx).toDouble();
            data.elevatorPosition = query.value(elevatorPositionIdx).toDouble();
            data.aileronPosition = query.value(aileronPositionIdx).toDouble();

            primaryFlightControlData.push_back(std::move(data));
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLitePrimaryFlightControlDao::getByAircraftId: SQL error" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return primaryFlightControlData;
}

bool SQLitePrimaryFlightControlDao::deleteByFlightId(std::int64_t flightId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   primary_flight_control "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLitePrimaryFlightControlDao::deleteByFlightId: SQL error" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLitePrimaryFlightControlDao::deleteByAircraftId(std::int64_t aircraftId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   primary_flight_control "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLitePrimaryFlightControlDao::deleteByAircraftId: SQL error" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return true;
}
