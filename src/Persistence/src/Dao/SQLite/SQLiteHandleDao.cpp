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
#include <iterator>
#include <cstdint>

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>

#include <Model/AircraftHandleData.h>
#include "SQLiteHandleDao.h"

// PUBLIC

SQLiteHandleDao::SQLiteHandleDao() noexcept
{}

SQLiteHandleDao::~SQLiteHandleDao() noexcept
{}

bool SQLiteHandleDao::add(std::int64_t aircraftId, const AircraftHandleData &aircraftHandleData)  noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into handle ("
        "  aircraft_id,"
        "  timestamp,"
        "  brake_left_position,"
        "  brake_right_position,"
        "  water_rudder_handle_position,"
        "  tail_hook_position,"
        "  canopy_open,"
        "  left_wing_folding,"
        "  right_wing_folding,"
        "  gear_handle_position,"
        "  smoke_enable"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :brake_left_position,"
        " :brake_right_position,"
        " :water_rudder_handle_position,"
        " :tail_hook_position,"
        " :canopy_open,"
        " :left_wing_folding,"
        " :right_wing_folding,"
        " :gear_handle_position,"
        " :smoke_enable"
        ");"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(aircraftHandleData.timestamp));
    query.bindValue(":brake_left_position", aircraftHandleData.brakeLeftPosition);
    query.bindValue(":brake_right_position", aircraftHandleData.brakeRightPosition);
    query.bindValue(":water_rudder_handle_position", aircraftHandleData.waterRudderHandlePosition);
    query.bindValue(":tail_hook_position", aircraftHandleData.tailhookPosition);
    query.bindValue(":canopy_open", aircraftHandleData.canopyOpen);
    query.bindValue(":left_wing_folding", aircraftHandleData.leftWingFolding);
    query.bindValue(":right_wing_folding", aircraftHandleData.rightWingFolding);
    query.bindValue(":gear_handle_position", aircraftHandleData.gearHandlePosition ? 1 : 0);
    query.bindValue(":smoke_enable", aircraftHandleData.smokeEnabled ? 1 : 0);

    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteHandleDao::add: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteHandleDao::getByAircraftId(std::int64_t aircraftId, std::back_insert_iterator<std::vector<AircraftHandleData>> backInsertIterator) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   handle h "
        "where  h.aircraft_id = :aircraft_id "
        "order by h.timestamp asc;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int timestampIdx = record.indexOf("timestamp");
        const int brakeLeftPositionIdx = record.indexOf("brake_left_position");
        const int brakeRightPositionIdx = record.indexOf("brake_right_position");
        const int waterRudderHandlePositionIdx = record.indexOf("water_rudder_handle_position");
        const int tailHookPositionIdx = record.indexOf("tail_hook_position");
        const int canopyOpenIdx = record.indexOf("canopy_open");
        const int leftWingFoldingIdx = record.indexOf("left_wing_folding");
        const int rightWingFoldingIdx = record.indexOf("right_wing_folding");
        const int gearHandlePositionIdx = record.indexOf("gear_handle_position");
        const int smokeEnablePositionIdx = record.indexOf("smoke_enable");
        while (query.next()) {

            AircraftHandleData data;

            data.timestamp = query.value(timestampIdx).toLongLong();
            data.brakeLeftPosition = query.value(brakeLeftPositionIdx).toInt();
            data.brakeRightPosition = query.value(brakeRightPositionIdx).toInt();
            data.waterRudderHandlePosition = query.value(waterRudderHandlePositionIdx).toInt();
            data.tailhookPosition = query.value(tailHookPositionIdx).toInt();
            data.canopyOpen = query.value(canopyOpenIdx).toInt();
            data.leftWingFolding = query.value(leftWingFoldingIdx).toInt();
            data.rightWingFolding = query.value(rightWingFoldingIdx).toInt();
            data.gearHandlePosition = query.value(gearHandlePositionIdx).toBool();
            data.smokeEnabled = query.value(smokeEnablePositionIdx).toBool();

            backInsertIterator = std::move(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteHandleDao::getByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }

    return ok;
}

bool SQLiteHandleDao::deleteByFlightId(std::int64_t flightId) noexcept
{
    QSqlQuery query;
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
        qDebug("SQLiteHandleDao::deleteByFlightId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteHandleDao::deleteByAircraftId(std::int64_t aircraftId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   handle "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteHandleDao::deleteByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return true;
}
