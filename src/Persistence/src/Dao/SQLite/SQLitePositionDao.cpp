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
#include <Model/PositionData.h>
#include "SQLitePositionDao.h"

namespace
{
    // The initial capacity of the position vector (e.g. SQLite does not support returning
    // the result count for the given SELECT query)
    // Samples at 30 Hz for an assumed flight duration of 2 * 60 seconds = 2 minutes
    constexpr int DefaultCapacity = 30 * 2 * 60;
}

struct SQLitePositionDaoPrivate
{
    SQLitePositionDaoPrivate(QString connectionName) noexcept
        : connectionName {std::move(connectionName)}
    {}

    QString connectionName;
};

// PUBLIC

SQLitePositionDao::SQLitePositionDao(QString connectionName) noexcept
    : d {std::make_unique<SQLitePositionDaoPrivate>(std::move(connectionName))}
{}

SQLitePositionDao::SQLitePositionDao(SQLitePositionDao &&rhs) noexcept = default;
SQLitePositionDao &SQLitePositionDao::operator=(SQLitePositionDao &&rhs) noexcept = default;
SQLitePositionDao::~SQLitePositionDao() = default;

bool SQLitePositionDao::add(std::int64_t aircraftId, const PositionData &position) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "insert into position ("
        "  aircraft_id,"
        "  timestamp,"
        "  latitude,"
        "  longitude,"
        "  altitude,"
        "  indicated_altitude"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :latitude,"
        " :longitude,"
        " :altitude,"
        " :indicated_altitude"
        ");"
    );
    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(position.timestamp));
    query.bindValue(":latitude", position.latitude);
    query.bindValue(":longitude", position.longitude);
    query.bindValue(":altitude", position.altitude);
    query.bindValue(":indicated_altitude", position.indicatedAltitude);

    const bool ok = query.exec();

#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLitePositionDao::add: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

std::vector<PositionData> SQLitePositionDao::getByAircraftId(std::int64_t aircraftId, bool *ok) const noexcept
{
    std::vector<PositionData> positionData;
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   position p "
        "where  p.aircraft_id = :aircraft_id "
        "order by p.timestamp asc;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool success = query.exec();
    if (success) {
        const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            positionData.reserve(query.size());
        } else {
            positionData.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const auto timestampIdx = record.indexOf("timestamp");
        const auto latitudeIdx = record.indexOf("latitude");
        const auto longitudeIdx = record.indexOf("longitude");
        const auto altitudeIdx = record.indexOf("altitude");
        const auto indicatedAltitudeIdx = record.indexOf("indicated_altitude");
        while (query.next()) {
            PositionData data;
            data.timestamp = query.value(timestampIdx).toLongLong();
            data.latitude = query.value(latitudeIdx).toDouble();
            data.longitude = query.value(longitudeIdx).toDouble();
            data.altitude = query.value(altitudeIdx).toDouble();
            data.indicatedAltitude = query.value(indicatedAltitudeIdx).toDouble();

            positionData.push_back(std::move(data));
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLitePositionDao::getByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return positionData;
}

bool SQLitePositionDao::deleteByFlightId(std::int64_t flightId) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   position "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLitePositionDao::deleteByFlightId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLitePositionDao::deleteByAircraftId(std::int64_t aircraftId) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   position "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLitePositionDao::deleteByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return true;
}
