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
#include <QObject>

#include "../../../../Kernel/src/Enum.h"
#include "../../../../Model/src/AircraftType.h"
#include "../../ConnectionManager.h"
#include "SQLiteAircraftTypeDao.h"

class SQLiteAircraftTypeDaoPrivate
{
public:
    SQLiteAircraftTypeDaoPrivate() noexcept
    {}

    std::unique_ptr<QSqlQuery> upsertQuery;

    void initQueries()
    {
        if (upsertQuery == nullptr) {
            upsertQuery = std::make_unique<QSqlQuery>();
            upsertQuery->prepare(
"insert into aircraft_type (title, category, wing_span, engine_type, nof_engines) "
"values('MyCraft', 'Boozo', 300, 2, 16) "
"on conflict(title) "
"do update "
"set category = excluded.category, "
"    wing_span = excluded.wing_span, "
"	 engine_type = excluded.engine_type, "
"	 nof_engines = excluded.nof_engines;"
") values ("
" :title,"
" :category,"
" :wing_span,"
" :engine_type,"
" :nof_engines"
");");
        }
    }

    void resetQueries() noexcept
    {
        upsertQuery = nullptr;

    }
};

// PUBLIC

SQLiteAircraftTypeDao::SQLiteAircraftTypeDao(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SQLiteAircraftTypeDaoPrivate>())
{
    frenchConnection();
}

SQLiteAircraftTypeDao::~SQLiteAircraftTypeDao() noexcept
{}

bool SQLiteAircraftTypeDao::upsert(const AircraftType &aircraftType)  noexcept
{
    d->initQueries();
    d->upsertQuery->bindValue(":type", aircraftType.type);
    d->upsertQuery->bindValue(":category", aircraftType.category);
    d->upsertQuery->bindValue(":wing_span", aircraftType.wingSpan);
    d->upsertQuery->bindValue(":engine_type", Enum::toUnderlyingType(aircraftType.engineType));
    d->upsertQuery->bindValue(":nof_engines", aircraftType.numberOfEngines);

    bool ok = d->upsertQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteAircraftTypeDao::upsert: SQL error: %s", qPrintable(d->upsertQuery->lastError().databaseText() + " - error code: " + d->upsertQuery->lastError().nativeErrorCode()));
#endif
    }
    return ok;
}

bool SQLiteAircraftTypeDao::getByType(const QString &type, AircraftType &aircraftType) const noexcept
{

}

bool SQLiteAircraftTypeDao::getAlld(std::insert_iterator<std::vector<AircraftType>> insertIterator) const noexcept
{

}

// PRIVATE

void SQLiteAircraftTypeDao::frenchConnection() noexcept
{
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &SQLiteAircraftTypeDao::handleConnectionChanged);
}

// PRIVATE SLOTS

void SQLiteAircraftTypeDao::handleConnectionChanged() noexcept
{
    d->resetQueries();
}
