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

#include "../../../../Kernel/src/Enum.h"
#include "../../PrimaryFlightControlData.h"
#include "SQLitePrimaryFlightControlDao.h"

class SQLitePrimaryFlightControlDaoPrivate
{
public:
    SQLitePrimaryFlightControlDaoPrivate() noexcept
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
"insert into primary_flight_control ("
"  aircraft_id,"
"  timestamp,"
"  yoke_x_position,"
"  yoke_y_position,"
"  rudder_position,"
"  elevator_position,"
"  aileron_position"
") values ("
" :aircraft_id,"
" :timestamp,"
" :yoke_x_position,"
" :yoke_y_position,"
" :rudder_position,"
" :elevator_position,"
" :aileron_position"
");");
        }
    }
};

// PUBLIC

SQLitePrimaryFlightControlDao::SQLitePrimaryFlightControlDao() noexcept
    : d(std::make_unique<SQLitePrimaryFlightControlDaoPrivate>())
{
}

SQLitePrimaryFlightControlDao::~SQLitePrimaryFlightControlDao() noexcept
{}

bool SQLitePrimaryFlightControlDao::addPrimaryFlightControl(qint64 aircraftId, const PrimaryFlightControlData &primaryFlightControlData)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId, QSql::In);
    d->insertQuery->bindValue(":timestamp", primaryFlightControlData.timestamp, QSql::In);
    d->insertQuery->bindValue(":yoke_x_position", primaryFlightControlData.yokeXPosition, QSql::In);
    d->insertQuery->bindValue(":yoke_y_position", primaryFlightControlData.yokeYPosition, QSql::In);
    d->insertQuery->bindValue(":rudder_position", primaryFlightControlData.rudderPosition, QSql::In);
    d->insertQuery->bindValue(":elevator_position", primaryFlightControlData.elevatorPosition, QSql::In);
    d->insertQuery->bindValue(":aileron_position", primaryFlightControlData.aileronPosition, QSql::In);

    bool ok = d->insertQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("addPrimaryFlightControl: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

PrimaryFlightControlData SQLitePrimaryFlightControlDao::getPrimaryFlightControl(qint64 aircraftId, qint64 timestamp) const noexcept
{
    return PrimaryFlightControlData();
}
