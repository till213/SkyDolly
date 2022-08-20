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
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/LightData.h>
#include "SQLiteLightDao.h"

// PUBLIC

SQLiteLightDao::SQLiteLightDao() noexcept
{}

SQLiteLightDao::~SQLiteLightDao() noexcept
{}

bool SQLiteLightDao::add(std::int64_t aircraftId, const LightData &lightData)  noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into light ("
        "  aircraft_id,"
        "  timestamp,"
        "  light_states"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :light_states"
        ");"
    );
    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(lightData.timestamp));
    query.bindValue(":light_states", static_cast<int>(lightData.lightStates));

    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteLightDao::add: SQL error" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteLightDao::getByAircraftId(std::int64_t aircraftId, std::back_insert_iterator<std::vector<LightData>> backInsertIterator) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   light l "
        "where  l.aircraft_id = :aircraft_id "
        "order by l.timestamp asc;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int timestampIdx = record.indexOf("timestamp");
        const int lightStatesIdx = record.indexOf("light_states");
        while (query.next()) {

            LightData data;
            data.timestamp = query.value(timestampIdx).toLongLong();
            data.lightStates = static_cast<SimType::LightStates>(query.value(lightStatesIdx).toInt());

            backInsertIterator = std::move(data);
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteLightDao::getByAircraftId: SQL error" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    return ok;
}

bool SQLiteLightDao::deleteByFlightId(std::int64_t flightId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   light "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    );
    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteLightDao::deleteByFlightId: SQL error" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteLightDao::deleteByAircraftId(std::int64_t aircraftId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   light "
        "where  aircraft_id = :aircraft_id;"
    );
    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteLightDao::deleteByAircraftId: SQL error" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return true;
}
