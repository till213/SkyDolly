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
#include <vector>
#include <iterator>

#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>

#include "../../../../Kernel/src/Enum.h"
#include "../../../../Model/src/AircraftType.h"
#include "SQLiteAircraftTypeDao.h"

// PUBLIC

SQLiteAircraftTypeDao::SQLiteAircraftTypeDao() noexcept
{}

SQLiteAircraftTypeDao::~SQLiteAircraftTypeDao() noexcept
{}

bool SQLiteAircraftTypeDao::upsert(const AircraftType &aircraftType)  noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into aircraft_type (type, category, wing_span, engine_type, nof_engines) "
        "values(:type, :category, :wing_span, :engine_type, :nof_engines) "
        "on conflict(type) "
        "do update "
        "set category = excluded.category, "
        "    wing_span = excluded.wing_span, "
        "    engine_type = excluded.engine_type, "
        "    nof_engines = excluded.nof_engines;"
    );

    query.bindValue(":type", aircraftType.type);
    query.bindValue(":category", aircraftType.category);
    query.bindValue(":wing_span", aircraftType.wingSpan);
    query.bindValue(":engine_type", Enum::toUnderlyingType(aircraftType.engineType));
    query.bindValue(":nof_engines", aircraftType.numberOfEngines);

    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteAircraftTypeDao::upsert: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteAircraftTypeDao::getByType(const QString &type, AircraftType &aircraftType) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select at.category, at.wing_span, at.engine_type, at.nof_engines "
        "from   aircraft_type at "
        "where  at.type = :type;"
    );

    query.bindValue(":type", type);

    bool ok = query.exec();
    if (ok) {
        const QSqlRecord record = query.record();
        const int categoryIdx = record.indexOf("category");
        const int wingSpanIdx = record.indexOf("wing_span");
        const int engineTypeIdx = record.indexOf("engine_type");
        const int nofEnginesIdx = record.indexOf("nof_engines");
        aircraftType.type = type;
        if (query.next()) {
            aircraftType.category = query.value(categoryIdx).toString();
            aircraftType.wingSpan = query.value(wingSpanIdx).toInt();
            aircraftType.engineType = static_cast<SimType::EngineType>(query.value(engineTypeIdx).toInt());
            aircraftType.numberOfEngines = query.value(nofEnginesIdx).toInt();
        }
    }
#ifdef DEBUG
    else {
        qDebug("SQLiteAircraftTypeDao::getByType: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteAircraftTypeDao::getAll(std::insert_iterator<std::vector<AircraftType>> insertIterator) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   aircraft_type at "
        "order by at.type asc;"
    );

    bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int typeIdx = record.indexOf("type");
        const int categoryIdx = record.indexOf("category");
        const int wingSpanIdx = record.indexOf("wing_span");
        const int engineTypeIdx = record.indexOf("engine_type");
        const int nofEnginesIdx = record.indexOf("nof_engines");
        while (query.next()) {
            AircraftType aircraftType;
            aircraftType.type = query.value(typeIdx).toString();
            aircraftType.category = query.value(categoryIdx).toString();
            aircraftType.wingSpan = query.value(wingSpanIdx).toInt();
            aircraftType.engineType = static_cast<SimType::EngineType>(query.value(engineTypeIdx).toInt());
            aircraftType.numberOfEngines = query.value(nofEnginesIdx).toInt();

            insertIterator = std::move(aircraftType);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLitePositionDao::getAlld: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }

    return ok;
}
