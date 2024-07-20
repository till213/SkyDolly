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
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDriver>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/AircraftHandleData.h>
#include "SQLiteHandleDao.h"

namespace
{
    // The initial capacity of the aircraft handles vector (e.g. SQLite does not support returning
    // the result count for the given SELECT query)
    constexpr int DefaultCapacity = 4;
}

struct SQLiteHandleDaoPrivate
{
    SQLiteHandleDaoPrivate(QString connectionName) noexcept
        : connectionName {std::move(connectionName)}
    {}

    QString connectionName;
};

// PUBLIC

SQLiteHandleDao::SQLiteHandleDao(QString connectionName) noexcept
    : d {std::make_unique<SQLiteHandleDaoPrivate>(std::move(connectionName))}
{}

SQLiteHandleDao::SQLiteHandleDao(SQLiteHandleDao &&rhs) noexcept = default;
SQLiteHandleDao &SQLiteHandleDao::operator=(SQLiteHandleDao &&rhs) noexcept = default;
SQLiteHandleDao::~SQLiteHandleDao() = default;

bool SQLiteHandleDao::add(std::int64_t aircraftId, const AircraftHandleData &aircraftHandleData) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "insert into handle ("
        "  aircraft_id,"
        "  timestamp,"
        "  brake_left_position,"
        "  brake_right_position,"
        "  steer_input_control,"
        "  water_rudder_handle_position,"
        "  tailhook_position,"
        "  canopy_open,"
        "  left_wing_folding,"
        "  right_wing_folding,"
        "  gear_handle_position,"
        "  tailhook_handle_position,"
        "  folding_wing_handle_position"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :brake_left_position,"
        " :brake_right_position,"
        " :steer_input_control,"
        " :water_rudder_handle_position,"
        " :tailhook_position,"
        " :canopy_open,"
        " :left_wing_folding,"
        " :right_wing_folding,"
        " :gear_handle_position,"
        " :tailhook_handle_position,"
        " :folding_wing_handle_position"
        ");"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(aircraftHandleData.timestamp));
    query.bindValue(":brake_left_position", aircraftHandleData.brakeLeftPosition);
    query.bindValue(":brake_right_position", aircraftHandleData.brakeRightPosition);
    query.bindValue(":steer_input_control", aircraftHandleData.steerInputControl);
    query.bindValue(":water_rudder_handle_position", aircraftHandleData.waterRudderHandlePosition);
    query.bindValue(":tailhook_position", aircraftHandleData.tailhookPosition);
    query.bindValue(":canopy_open", aircraftHandleData.canopyOpen);
    query.bindValue(":left_wing_folding", aircraftHandleData.leftWingFolding);
    query.bindValue(":right_wing_folding", aircraftHandleData.rightWingFolding);
    query.bindValue(":gear_handle_position", aircraftHandleData.gearHandlePosition ? 1 : 0);
    query.bindValue(":tailhook_handle_position", aircraftHandleData.tailhookHandlePosition ? 1 : 0);
    query.bindValue(":folding_wing_handle_position", aircraftHandleData.foldingWingHandlePosition ? 1 : 0);

    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteHandleDao::add: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

std::vector<AircraftHandleData> SQLiteHandleDao::getByAircraftId(std::int64_t aircraftId, bool *ok) const noexcept
{
    std::vector<AircraftHandleData> aircraftHandleData;
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   handle h "
        "where  h.aircraft_id = :aircraft_id "
        "order by h.timestamp asc;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool success = query.exec();
    if (success) {
        const auto db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            aircraftHandleData.reserve(query.size());
        } else {
            aircraftHandleData.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const auto timestampIdx = record.indexOf("timestamp");
        const auto brakeLeftPositionIdx = record.indexOf("brake_left_position");
        const auto brakeRightPositionIdx = record.indexOf("brake_right_position");
        const auto steerInputControlIdx = record.indexOf("steer_input_control");
        const auto waterRudderHandlePositionIdx = record.indexOf("water_rudder_handle_position");
        const auto tailhookPositionIdx = record.indexOf("tailhook_position");
        const auto canopyOpenIdx = record.indexOf("canopy_open");
        const auto leftWingFoldingIdx = record.indexOf("left_wing_folding");
        const auto rightWingFoldingIdx = record.indexOf("right_wing_folding");
        const auto gearHandlePositionIdx = record.indexOf("gear_handle_position");
        const auto tailhookHandlePositionIdx = record.indexOf("tailhook_handle_position");
        const auto foldingWingHandlePositionIdx = record.indexOf("folding_wing_handle_position");
        while (query.next()) {
            AircraftHandleData data;
            data.timestamp = query.value(timestampIdx).toLongLong();
            data.brakeLeftPosition = static_cast<std::int16_t>(query.value(brakeLeftPositionIdx).toInt());
            data.brakeRightPosition = static_cast<std::int16_t>(query.value(brakeRightPositionIdx).toInt());
            data.steerInputControl = static_cast<std::int16_t>(query.value(steerInputControlIdx).toInt());
            data.waterRudderHandlePosition = static_cast<std::int16_t>(query.value(waterRudderHandlePositionIdx).toInt());
            data.tailhookPosition = query.value(tailhookPositionIdx).toInt();
            data.canopyOpen = query.value(canopyOpenIdx).toInt();
            data.leftWingFolding = query.value(leftWingFoldingIdx).toInt();
            data.rightWingFolding = query.value(rightWingFoldingIdx).toInt();
            data.gearHandlePosition = query.value(gearHandlePositionIdx).toBool();
            data.tailhookHandlePosition = query.value(tailhookHandlePositionIdx).toBool();
            data.foldingWingHandlePosition = query.value(foldingWingHandlePositionIdx).toBool();

            aircraftHandleData.push_back(std::move(data));
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteHandleDao::getByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftHandleData;
}

bool SQLiteHandleDao::deleteByFlightId(std::int64_t flightId) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   handle "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteHandleDao::deleteByFlightId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteHandleDao::deleteByAircraftId(std::int64_t aircraftId) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   handle "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteHandleDao::deleteByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return true;
}
