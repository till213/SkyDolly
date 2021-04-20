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

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>

#include "../../AircraftHandleData.h"
#include "SQLiteHandleDao.h"

class SQLiteHandleDaoPrivate
{
public:
    SQLiteHandleDaoPrivate() noexcept
        : insertQuery(nullptr),
          selectQuery(nullptr)
    {
    }

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectQuery;

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
"  gear_handle_position,"
"  folding_wing_handle_position"
") values ("
" :aircraft_id,"
" :timestamp,"
" :brake_left_position,"
" :brake_right_position,"
" :water_rudder_handle_position,"
" :tail_hook_position,"
" :canopy_open,"
" :gear_handle_position,"
" :folding_wing_handle_position"
");");           
        }
        if (selectQuery == nullptr) {
            selectQuery = std::make_unique<QSqlQuery>();
            selectQuery->prepare("select a.name from handle a where a.id = :id;");
        }
    }
};

// PUBLIC

SQLiteHandleDao::SQLiteHandleDao() noexcept
    : d(std::make_unique<SQLiteHandleDaoPrivate>())
{
}

SQLiteHandleDao::~SQLiteHandleDao() noexcept
{}

bool SQLiteHandleDao::addHandle(qint64 aircraftId, const AircraftHandleData &aircraftHandleData)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId, QSql::In);
    d->insertQuery->bindValue(":timestamp", aircraftHandleData.timestamp, QSql::In);
    d->insertQuery->bindValue(":brake_left_position", aircraftHandleData.brakeLeftPosition, QSql::In);
    d->insertQuery->bindValue(":brake_right_position", aircraftHandleData.brakeRightPosition, QSql::In);
    d->insertQuery->bindValue(":water_rudder_handle_position", aircraftHandleData.waterRudderHandlePosition, QSql::In);
    d->insertQuery->bindValue(":tail_hook_position", aircraftHandleData.tailhookPosition, QSql::In);
    d->insertQuery->bindValue(":canopy_open", aircraftHandleData.canopyOpen, QSql::In);
    d->insertQuery->bindValue(":gear_handle_position", aircraftHandleData.gearHandlePosition ? 1 : 0, QSql::In);
    d->insertQuery->bindValue(":folding_wing_handle_position", aircraftHandleData.foldingWingHandlePosition, QSql::In);

    bool ok = d->insertQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("addHandle: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

AircraftHandleData SQLiteHandleDao::getHandle(qint64 aircraftId, qint64 timestamp) const noexcept
{
    return AircraftHandleData();
}
