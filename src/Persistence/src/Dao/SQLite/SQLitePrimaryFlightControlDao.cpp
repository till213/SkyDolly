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
#include <Model/PrimaryFlightControlData.h>
#include "SQLitePrimaryFlightControlDao.h"

namespace
{
    // The initial capacity of the primary flight control vector (e.g. SQLite does not support returning
    // the result count for the given SELECT query)
    // Samples at 30 Hz for an assumed flight duration of 2 * 60 seconds = 2 minutes
    constexpr int DefaultCapacity = 30 * 2 * 60;
}

struct SQLitePrimaryFlightControlDaoPrivate
{
    SQLitePrimaryFlightControlDaoPrivate(QString connectionName) noexcept
        : connectionName(std::move(connectionName))
    {}

    QString connectionName;
};

// PUBLIC

SQLitePrimaryFlightControlDao::SQLitePrimaryFlightControlDao(QString connectionName) noexcept
    : d(std::make_unique<SQLitePrimaryFlightControlDaoPrivate>(std::move(connectionName)))
{}

SQLitePrimaryFlightControlDao::SQLitePrimaryFlightControlDao(SQLitePrimaryFlightControlDao &&rhs) noexcept = default;
SQLitePrimaryFlightControlDao &SQLitePrimaryFlightControlDao::operator=(SQLitePrimaryFlightControlDao &&rhs) noexcept = default;
SQLitePrimaryFlightControlDao::~SQLitePrimaryFlightControlDao() = default;

bool SQLitePrimaryFlightControlDao::add(std::int64_t aircraftId, const PrimaryFlightControlData &primaryFlightControlData) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "insert into primary_flight_control ("
        "  aircraft_id,"
        "  timestamp,"
        "  rudder_deflection,"
        "  elevator_deflection,"
        "  aileron_left_deflection,"
        "  aileron_right_deflection,"
        "  rudder_position,"
        "  elevator_position,"
        "  aileron_position"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :rudder_deflection,"
        " :elevator_deflection,"
        " :aileron_left_deflection,"
        " :aileron_right_deflection,"
        " :rudder_position,"
        " :elevator_position,"
        " :aileron_position"
        ");"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(primaryFlightControlData.timestamp));
    query.bindValue(":rudder_deflection", primaryFlightControlData.rudderDeflection);
    query.bindValue(":elevator_deflection", primaryFlightControlData.elevatorDeflection);
    query.bindValue(":aileron_left_deflection", primaryFlightControlData.leftAileronDeflection);
    query.bindValue(":aileron_right_deflection", primaryFlightControlData.rightAileronDeflection);
    query.bindValue(":rudder_position", primaryFlightControlData.rudderPosition);
    query.bindValue(":elevator_position", primaryFlightControlData.elevatorPosition);
    query.bindValue(":aileron_position", primaryFlightControlData.aileronPosition);

    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLitePrimaryFlightControlDao::add: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

std::vector<PrimaryFlightControlData> SQLitePrimaryFlightControlDao::getByAircraftId(std::int64_t aircraftId, bool *ok) const noexcept
{
    std::vector<PrimaryFlightControlData> primaryFlightControlData;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
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
        QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            primaryFlightControlData.reserve(query.size());
        } else {
            primaryFlightControlData.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const int timestampIdx = record.indexOf("timestamp");
        const int rudderDeflectionIdx = record.indexOf("rudder_deflection");
        const int elevatorDeflectionIdx = record.indexOf("elevator_deflection");
        const int aileronLeftDeflectionIdx = record.indexOf("aileron_left_deflection");
        const int aileronRightDeflectionIdx = record.indexOf("aileron_right_deflection");
        const int rudderPositionIdx = record.indexOf("rudder_position");
        const int elevatorPositionIdx = record.indexOf("elevator_position");
        const int aileronPositionIdx = record.indexOf("aileron_position");
        while (query.next()) {

            PrimaryFlightControlData data;

            data.timestamp = query.value(timestampIdx).toLongLong();
            data.rudderDeflection = query.value(rudderDeflectionIdx).toFloat();
            data.elevatorDeflection = query.value(elevatorDeflectionIdx).toFloat();
            data.leftAileronDeflection = query.value(aileronLeftDeflectionIdx).toFloat();
            data.rightAileronDeflection = query.value(aileronRightDeflectionIdx).toFloat();
            data.rudderPosition = static_cast<std::int16_t>(query.value(rudderPositionIdx).toInt());
            data.elevatorPosition = static_cast<std::int16_t>(query.value(elevatorPositionIdx).toInt());
            data.aileronPosition = static_cast<std::int16_t>(query.value(aileronPositionIdx).toInt());

            primaryFlightControlData.push_back(std::move(data));
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLitePrimaryFlightControlDao::getByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return primaryFlightControlData;
}

bool SQLitePrimaryFlightControlDao::deleteByFlightId(std::int64_t flightId) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
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
        qDebug() << "SQLitePrimaryFlightControlDao::deleteByFlightId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLitePrimaryFlightControlDao::deleteByAircraftId(std::int64_t aircraftId) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   primary_flight_control "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLitePrimaryFlightControlDao::deleteByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return true;
}
