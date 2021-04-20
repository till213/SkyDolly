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
#include "../../SecondaryFlightControlData.h"
#include "SQLiteSecondaryFlightControlDao.h"

class SQLiteSecondaryFlightControlDaoPrivate
{
public:
    SQLiteSecondaryFlightControlDaoPrivate() noexcept
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
"insert into secondary_flight_control ("
"  aircraft_id,"
"  timestamp,"
"  leading_edge_flaps_left_percent,"
"  leading_edge_flaps_right_percent,"
"  trailing_edge_flaps_left_percent,"
"  trailing_edge_flaps_right_percent,"
"  spoilers_handle_position,"
"  flaps_handle_index"
") values ("
" :aircraft_id,"
" :timestamp,"
" :leading_edge_flaps_left_percent,"
" :leading_edge_flaps_right_percent,"
" :trailing_edge_flaps_left_percent,"
" :trailing_edge_flaps_right_percent,"
" :spoilers_handle_position,"
" :flaps_handle_index"
");");
        }
        if (selectQuery == nullptr) {
            selectQuery = std::make_unique<QSqlQuery>();
            selectQuery->prepare("select a.name from aircraft a where a.id = :id;");
        }
    }
};

// PUBLIC

SQLiteSecondaryFlightControlDao::SQLiteSecondaryFlightControlDao() noexcept
    : d(std::make_unique<SQLiteSecondaryFlightControlDaoPrivate>())
{
}

SQLiteSecondaryFlightControlDao::~SQLiteSecondaryFlightControlDao() noexcept
{}

bool SQLiteSecondaryFlightControlDao::addSecondaryFlightControl(qint64 aircraftId, const SecondaryFlightControlData &secondaryFlightControlData)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId, QSql::In);
    d->insertQuery->bindValue(":timestamp", secondaryFlightControlData.timestamp, QSql::In);
    d->insertQuery->bindValue(":leading_edge_flaps_left_percent", secondaryFlightControlData.leadingEdgeFlapsLeftPercent, QSql::In);
    d->insertQuery->bindValue(":leading_edge_flaps_right_percent", secondaryFlightControlData.leadingEdgeFlapsRightPercent, QSql::In);
    d->insertQuery->bindValue(":trailing_edge_flaps_left_percent", secondaryFlightControlData.trailingEdgeFlapsLeftPercent, QSql::In);
    d->insertQuery->bindValue(":trailing_edge_flaps_right_percent", secondaryFlightControlData.trailingEdgeFlapsRightPercent, QSql::In);
    d->insertQuery->bindValue(":spoilers_handle_position", secondaryFlightControlData.spoilersHandlePosition, QSql::In);
    d->insertQuery->bindValue(":flaps_handle_index", secondaryFlightControlData.flapsHandleIndex, QSql::In);

    bool ok = d->insertQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("addSecondaryFlightControl: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

SecondaryFlightControlData SQLiteSecondaryFlightControlDao::getSecondaryFlightControl(qint64 aircraftId, qint64 timestamp) const noexcept
{
    return SecondaryFlightControlData();
}
