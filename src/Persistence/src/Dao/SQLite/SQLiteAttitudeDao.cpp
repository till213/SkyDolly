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
#include <cstdint>
#include <utility>

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDriver>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Model/AttitudeData.h>
#include "SQLiteAttitudeDao.h"

namespace
{
    // The initial capacity of the attitude vector (e.g. SQLite does not support returning
    // the result count for the given SELECT query)
    // Samples at 30 Hz for an assumed flight duration of 2 * 60 seconds = 2 minutes
    constexpr int DefaultCapacity = 30 * 2 * 60;
}

struct SQLiteAttitudeDaoPrivate
{
    SQLiteAttitudeDaoPrivate(QString connectionName) noexcept
        : connectionName {std::move(connectionName)}
    {}

    QString connectionName;
};

// PUBLIC

SQLiteAttitudeDao::SQLiteAttitudeDao(QString connectionName) noexcept
    : d {std::make_unique<SQLiteAttitudeDaoPrivate>(std::move(connectionName))}
{}

SQLiteAttitudeDao::SQLiteAttitudeDao(SQLiteAttitudeDao &&rhs) noexcept = default;
SQLiteAttitudeDao &SQLiteAttitudeDao::operator=(SQLiteAttitudeDao &&rhs) noexcept = default;
SQLiteAttitudeDao::~SQLiteAttitudeDao() = default;

bool SQLiteAttitudeDao::add(std::int64_t aircraftId, const AttitudeData &attitude) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "insert into attitude ("
        "  aircraft_id,"
        "  timestamp,"
        "  pitch,"
        "  bank,"
        "  true_heading,"
        "  velocity_x,"
        "  velocity_y,"
        "  velocity_z,"
        "  on_ground"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :pitch,"
        " :bank,"
        " :true_heading,"
        " :velocity_x,"
        " :velocity_y,"
        " :velocity_z,"
        " :on_ground"
        ");"
    );
    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(attitude.timestamp));
    query.bindValue(":pitch", attitude.pitch);
    query.bindValue(":bank", attitude.bank);
    query.bindValue(":true_heading", attitude.trueHeading);
    query.bindValue(":velocity_x", attitude.velocityBodyX);
    query.bindValue(":velocity_y", attitude.velocityBodyY);
    query.bindValue(":velocity_z", attitude.velocityBodyZ);
    query.bindValue(":on_ground", attitude.onGround);

    const bool ok = query.exec();

#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteAttitudeDao::add: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

std::vector<AttitudeData> SQLiteAttitudeDao::getByAircraftId(std::int64_t aircraftId, bool *ok) const noexcept
{
    std::vector<AttitudeData> attitudeData;
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   attitude a "
        "where  a.aircraft_id = :aircraft_id "
        "order by a.timestamp asc;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool success = query.exec();
    if (success) {
        const auto db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            attitudeData.reserve(query.size());
        } else {
            attitudeData.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const auto timestampIdx = record.indexOf("timestamp");
        const auto pitchIdx = record.indexOf("pitch");
        const auto bankIdx = record.indexOf("bank");
        const auto trueHeadingIdx = record.indexOf("true_heading");
        const auto velocitXIdx = record.indexOf("velocity_x");
        const auto velocitYIdx = record.indexOf("velocity_y");
        const auto velocitZIdx = record.indexOf("velocity_z");
        const auto onGroundIdx = record.indexOf("on_ground");
        while (query.next()) {
            AttitudeData data;
            data.timestamp = query.value(timestampIdx).toLongLong();
            data.pitch = query.value(pitchIdx).toDouble();
            data.bank = query.value(bankIdx).toDouble();
            data.trueHeading = query.value(trueHeadingIdx).toDouble();
            data.velocityBodyX = query.value(velocitXIdx).toDouble();
            data.velocityBodyY = query.value(velocitYIdx).toDouble();
            data.velocityBodyZ = query.value(velocitZIdx).toDouble();
            data.onGround = query.value(onGroundIdx).toBool();

            attitudeData.push_back(std::move(data));
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteAttitudeDao::getByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return attitudeData;
}

bool SQLiteAttitudeDao::deleteByFlightId(std::int64_t flightId) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   attitude "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteAttitudeDao::deleteByFlightId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteAttitudeDao::deleteByAircraftId(std::int64_t aircraftId) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   attitude "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteAttitudeDao::deleteByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return true;
}
