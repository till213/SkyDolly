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

#include <QObject>
#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>

#include "../../../../Model/src/AircraftHandleData.h"
#include "../../ConnectionManager.h"
#include "SQLiteHandleDao.h"

class SQLiteHandleDaoPrivate
{
public:
    SQLiteHandleDaoPrivate() noexcept
    {}

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByAircraftIdQuery;
    std::unique_ptr<QSqlQuery> deleteByFlightIdQuery;
    std::unique_ptr<QSqlQuery> deleteByIdQuery;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();
            insertQuery->prepare(
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
"  gear_handle_position"
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
" :gear_handle_position"
");");           
        }
        if (selectByAircraftIdQuery == nullptr) {
            selectByAircraftIdQuery = std::make_unique<QSqlQuery>();
            selectByAircraftIdQuery->prepare(
"select * "
"from   handle h "
"where  h.aircraft_id = :aircraft_id "
"order by h.timestamp asc;");
        }
        if (deleteByFlightIdQuery == nullptr) {
            deleteByFlightIdQuery = std::make_unique<QSqlQuery>();
            deleteByFlightIdQuery->prepare(
"delete "
"from   handle "
"where  aircraft_id in (select a.id "
"                       from aircraft a"
"                       where a.flight_id = :flight_id"
"                      );");
        }
        if (deleteByIdQuery == nullptr) {
            deleteByIdQuery = std::make_unique<QSqlQuery>();
            deleteByIdQuery->prepare(
"delete "
"from   handle "
"where  aircraft_id = :aircraft_id;");
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

SQLiteHandleDao::SQLiteHandleDao(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SQLiteHandleDaoPrivate>())
{
    frenchConnection();
}

SQLiteHandleDao::~SQLiteHandleDao() noexcept
{}

bool SQLiteHandleDao::add(qint64 aircraftId, const AircraftHandleData &aircraftHandleData)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId);
    d->insertQuery->bindValue(":timestamp", aircraftHandleData.timestamp);
    d->insertQuery->bindValue(":brake_left_position", aircraftHandleData.brakeLeftPosition);
    d->insertQuery->bindValue(":brake_right_position", aircraftHandleData.brakeRightPosition);
    d->insertQuery->bindValue(":water_rudder_handle_position", aircraftHandleData.waterRudderHandlePosition);
    d->insertQuery->bindValue(":tail_hook_position", aircraftHandleData.tailhookPosition);
    d->insertQuery->bindValue(":canopy_open", aircraftHandleData.canopyOpen);
    d->insertQuery->bindValue(":left_wing_folding", aircraftHandleData.leftWingFolding);
    d->insertQuery->bindValue(":right_wing_folding", aircraftHandleData.rightWingFolding);
    d->insertQuery->bindValue(":gear_handle_position", aircraftHandleData.gearHandlePosition ? 1 : 0);    

    bool ok = d->insertQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteHandleDao::add: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteHandleDao::getByAircraftId(qint64 aircraftId, QVector<AircraftHandleData> &aircraftHandleData) const noexcept
{
    d->initQueries();
    d->selectByAircraftIdQuery->bindValue(":aircraft_id", aircraftId);
    bool ok = d->selectByAircraftIdQuery->exec();
    if (ok) {
        aircraftHandleData.clear();
        QSqlRecord record = d->selectByAircraftIdQuery->record();
        const int timestampIdx = record.indexOf("timestamp");
        const int brakeLeftPositionIdx = record.indexOf("brake_left_position");
        const int brakeRightPositionIdx = record.indexOf("brake_right_position");
        const int waterRudderHandlePositionIdx = record.indexOf("water_rudder_handle_position");
        const int tailHookPositionIdx = record.indexOf("tail_hook_position");
        const int canopyOpenIdx = record.indexOf("canopy_open");
        const int leftWingFoldingIdx = record.indexOf("left_wing_folding");
        const int rightWingFoldingIdx = record.indexOf("right_wing_folding");
        const int gearHandlePositionIdx = record.indexOf("gear_handle_position");
        while (d->selectByAircraftIdQuery->next()) {

            AircraftHandleData data;

            data.timestamp = d->selectByAircraftIdQuery->value(timestampIdx).toLongLong();
            data.brakeLeftPosition = d->selectByAircraftIdQuery->value(brakeLeftPositionIdx).toInt();
            data.brakeRightPosition = d->selectByAircraftIdQuery->value(brakeRightPositionIdx).toInt();
            data.waterRudderHandlePosition = d->selectByAircraftIdQuery->value(waterRudderHandlePositionIdx).toInt();
            data.tailhookPosition = d->selectByAircraftIdQuery->value(tailHookPositionIdx).toInt();
            data.canopyOpen = d->selectByAircraftIdQuery->value(canopyOpenIdx).toInt();
            data.leftWingFolding = d->selectByAircraftIdQuery->value(leftWingFoldingIdx).toInt();
            data.rightWingFolding = d->selectByAircraftIdQuery->value(rightWingFoldingIdx).toInt();
            data.gearHandlePosition = d->selectByAircraftIdQuery->value(gearHandlePositionIdx).toBool();            

            aircraftHandleData.append(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteHandleDao::getByAircraftId: SQL error: %s", qPrintable(d->selectByAircraftIdQuery->lastError().databaseText() + " - error code: " + d->selectByAircraftIdQuery->lastError().nativeErrorCode()));
#endif
    }

    return ok;
}

bool SQLiteHandleDao::deleteByFlightId(qint64 flightId) noexcept
{
    d->initQueries();
    d->deleteByFlightIdQuery->bindValue(":flight_id", flightId);
    bool ok = d->deleteByFlightIdQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteHandleDao::deleteByFlightId: SQL error: %s", qPrintable(d->deleteByFlightIdQuery->lastError().databaseText() + " - error code: " + d->deleteByFlightIdQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteHandleDao::deleteByAircraftId(qint64 aircraftId) noexcept
{
    d->initQueries();
    d->deleteByIdQuery->bindValue(":aircraft_id", aircraftId);
    bool ok = d->deleteByIdQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteHandleDao::deleteByAircraftId: SQL error: %s", qPrintable(d->deleteByIdQuery->lastError().databaseText() + " - error code: " + d->deleteByIdQuery->lastError().nativeErrorCode()));
    }
#endif
    return true;
}

// PRIVATE

void SQLiteHandleDao::frenchConnection() noexcept
{
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &SQLiteHandleDao::handleConnectionChanged);
}

// PRIVATE SLOTS

void SQLiteHandleDao::handleConnectionChanged() noexcept
{
    d->resetQueries();
}
