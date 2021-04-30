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

#include "../../../../Model/src/LightData.h"
#include "../../ConnectionManager.h"
#include "SQLiteLightDao.h"

class SQLiteLightDaoPrivate
{
public:
    SQLiteLightDaoPrivate() noexcept
    {}

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByAircraftIdQuery;
    std::unique_ptr<QSqlQuery> deleteByScenarioIdQuery;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();
            insertQuery->prepare(
"insert into light ("
"  aircraft_id,"
"  timestamp,"
"  light_states"
") values ("
" :aircraft_id,"
" :timestamp,"
" :light_states"
");");
        }
        if (selectByAircraftIdQuery == nullptr) {
            selectByAircraftIdQuery = std::make_unique<QSqlQuery>();
            selectByAircraftIdQuery->prepare(
"select * "
"from   light l "
"where  l.aircraft_id = :aircraft_id "
"order by l.timestamp asc;");
        }
        if (deleteByScenarioIdQuery == nullptr) {
            deleteByScenarioIdQuery = std::make_unique<QSqlQuery>();
            deleteByScenarioIdQuery->prepare(
"delete "
"from   light "
"where  aircraft_id in (select a.id "
"                       from aircraft a"
"                       where a.scenario_id = :scenario_id"
"                      );");
        }
    }

    void resetQueries() noexcept
    {
        insertQuery = nullptr;
        selectByAircraftIdQuery = nullptr;
        deleteByScenarioIdQuery = nullptr;
    }
};

// PUBLIC

SQLiteLightDao::SQLiteLightDao(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SQLiteLightDaoPrivate>())
{
    frenchConnection();
}

SQLiteLightDao::~SQLiteLightDao() noexcept
{}

bool SQLiteLightDao::add(qint64 aircraftId, const LightData &lightData)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId);
    d->insertQuery->bindValue(":timestamp", lightData.timestamp);
    d->insertQuery->bindValue(":light_states", static_cast<int>(lightData.lightStates));

    bool ok = d->insertQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteLightDao::add: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteLightDao::getByAircraftId(qint64 aircraftId, QVector<LightData> &lightData) const noexcept
{
    d->initQueries();
    d->selectByAircraftIdQuery->bindValue(":aircraft_id", aircraftId);
    bool ok = d->selectByAircraftIdQuery->exec();
    if (ok) {
        lightData.clear();
        int timestampIdx = d->selectByAircraftIdQuery->record().indexOf("timestamp");
        int lightStatesIdx = d->selectByAircraftIdQuery->record().indexOf("light_states");
        while (d->selectByAircraftIdQuery->next()) {

            LightData data;
            data.timestamp = d->selectByAircraftIdQuery->value(timestampIdx).toLongLong();
            data.lightStates = static_cast<SimType::LightStates>(d->selectByAircraftIdQuery->value(lightStatesIdx).toInt());
            lightData.append(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteLightDao::getByAircraftId: SQL error: %s", qPrintable(d->selectByAircraftIdQuery->lastError().databaseText() + " - error code: " + d->selectByAircraftIdQuery->lastError().nativeErrorCode()));
#endif
    }

    return ok;
}

bool SQLiteLightDao::deleteByScenarioId(qint64 scenarioId) noexcept
{
    d->initQueries();
    d->deleteByScenarioIdQuery->bindValue(":scenario_id", scenarioId);
    bool ok = d->deleteByScenarioIdQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteLightDao::deleteByScenarioId: SQL error: %s", qPrintable(d->deleteByScenarioIdQuery->lastError().databaseText() + " - error code: " + d->deleteByScenarioIdQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

// PRIVATE

void SQLiteLightDao::frenchConnection() noexcept
{
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &SQLiteLightDao::handleConnectionChanged);
}

// PRIVATE SLOTS

void SQLiteLightDao::handleConnectionChanged() noexcept
{
    d->resetQueries();
}


