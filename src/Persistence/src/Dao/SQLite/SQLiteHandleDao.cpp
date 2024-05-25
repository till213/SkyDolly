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
    : d(std::make_unique<SQLiteHandleDaoPrivate>(std::move(connectionName)))
{}

SQLiteHandleDao::SQLiteHandleDao(SQLiteHandleDao &&rhs) noexcept = default;
SQLiteHandleDao &SQLiteHandleDao::operator=(SQLiteHandleDao &&rhs) noexcept = default;
SQLiteHandleDao::~SQLiteHandleDao() = default;

bool SQLiteHandleDao::add(std::int64_t aircraftId, const AircraftHandleData &aircraftHandleData) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(QStringLiteral(
        "insert into handle ("
        "  aircraft_id,"
        "  timestamp,"
        "  brake_left_position,"
        "  brake_right_position,"
        "  water_rudder_handle_position,"
        "  tailhook_position,"
        "  canopy_open,"
        "  left_wing_folding,"
        "  right_wing_folding,"
        "  gear_handle_position,"
        "  tailhook_handle_position,"
        "  folding_wing_handle_position,"
        "  smoke_enable"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :brake_left_position,"
        " :brake_right_position,"
        " :water_rudder_handle_position,"
        " :tailhook_position,"
        " :canopy_open,"
        " :left_wing_folding,"
        " :right_wing_folding,"
        " :gear_handle_position,"
        " :tailhook_handle_position,"
        " :folding_wing_handle_position,"
        " :smoke_enable"
        ");"
    ));

    query.bindValue(QStringLiteral(":aircraft_id"), QVariant::fromValue(aircraftId));
    query.bindValue(QStringLiteral(":timestamp"), QVariant::fromValue(aircraftHandleData.timestamp));
    query.bindValue(QStringLiteral(":brake_left_position"), aircraftHandleData.brakeLeftPosition);
    query.bindValue(QStringLiteral(":brake_right_position"), aircraftHandleData.brakeRightPosition);
    query.bindValue(QStringLiteral(":water_rudder_handle_position"), aircraftHandleData.waterRudderHandlePosition);
    query.bindValue(QStringLiteral(":tailhook_position"), aircraftHandleData.tailhookPosition);
    query.bindValue(QStringLiteral(":canopy_open"), aircraftHandleData.canopyOpen);
    query.bindValue(QStringLiteral(":left_wing_folding"), aircraftHandleData.leftWingFolding);
    query.bindValue(QStringLiteral(":right_wing_folding"), aircraftHandleData.rightWingFolding);
    query.bindValue(QStringLiteral(":gear_handle_position"), aircraftHandleData.gearHandlePosition ? 1 : 0);
    query.bindValue(QStringLiteral(":tailhook_handle_position"), aircraftHandleData.tailhookHandlePosition ? 1 : 0);
    query.bindValue(QStringLiteral(":folding_wing_handle_position"), aircraftHandleData.foldingWingHandlePosition ? 1 : 0);
    query.bindValue(QStringLiteral(":smoke_enable"), aircraftHandleData.smokeEnabled ? 1 : 0);

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
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(
        "select * "
        "from   handle h "
        "where  h.aircraft_id = :aircraft_id "
        "order by h.timestamp asc;"
    ));

    query.bindValue(QStringLiteral(":aircraft_id"), QVariant::fromValue(aircraftId));
    const bool success = query.exec();
    if (success) {
        const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            aircraftHandleData.reserve(query.size());
        } else {
            aircraftHandleData.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const int timestampIdx = record.indexOf(QStringLiteral("timestamp"));
        const int brakeLeftPositionIdx = record.indexOf(QStringLiteral("brake_left_position"));
        const int brakeRightPositionIdx = record.indexOf(QStringLiteral("brake_right_position"));
        const int waterRudderHandlePositionIdx = record.indexOf(QStringLiteral("water_rudder_handle_position"));
        const int tailhookPositionIdx = record.indexOf(QStringLiteral("tailhook_position"));
        const int canopyOpenIdx = record.indexOf(QStringLiteral("canopy_open"));
        const int leftWingFoldingIdx = record.indexOf(QStringLiteral("left_wing_folding"));
        const int rightWingFoldingIdx = record.indexOf(QStringLiteral("right_wing_folding"));
        const int gearHandlePositionIdx = record.indexOf(QStringLiteral("gear_handle_position"));
        const int tailhookHandlePositionIdx = record.indexOf(QStringLiteral("tailhook_handle_position"));
        const int foldingWingHandlePositionIdx = record.indexOf(QStringLiteral("folding_wing_handle_position"));
        const int smokeEnablePositionIdx = record.indexOf(QStringLiteral("smoke_enable"));
        while (query.next()) {
            AircraftHandleData data;
            data.timestamp = query.value(timestampIdx).toLongLong();
            data.brakeLeftPosition = static_cast<std::int16_t>(query.value(brakeLeftPositionIdx).toInt());
            data.brakeRightPosition = static_cast<std::int16_t>(query.value(brakeRightPositionIdx).toInt());
            data.waterRudderHandlePosition = static_cast<std::int16_t>(query.value(waterRudderHandlePositionIdx).toInt());
            data.tailhookPosition = query.value(tailhookPositionIdx).toInt();
            data.canopyOpen = query.value(canopyOpenIdx).toInt();
            data.leftWingFolding = query.value(leftWingFoldingIdx).toInt();
            data.rightWingFolding = query.value(rightWingFoldingIdx).toInt();
            data.gearHandlePosition = query.value(gearHandlePositionIdx).toBool();
            data.tailhookHandlePosition = query.value(tailhookHandlePositionIdx).toBool();
            data.foldingWingHandlePosition = query.value(foldingWingHandlePositionIdx).toBool();
            data.smokeEnabled = query.value(smokeEnablePositionIdx).toBool();

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
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(QStringLiteral(
        "delete "
        "from   handle "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    ));

    query.bindValue(QStringLiteral(":flight_id"), QVariant::fromValue(flightId));
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
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(QStringLiteral(
        "delete "
        "from   handle "
        "where  aircraft_id = :aircraft_id;"
    ));

    query.bindValue(QStringLiteral(":aircraft_id"), QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteHandleDao::deleteByAircraftId: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return true;
}
