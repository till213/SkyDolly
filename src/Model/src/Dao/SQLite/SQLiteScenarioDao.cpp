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
#include "../../Scenario.h"
#include "../../FlightCondition.h"
#include "SQLiteScenarioDao.h"

class SQLiteScenarioDaoPrivate
{
public:
    SQLiteScenarioDaoPrivate() noexcept
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
"insert into scenario ("
"  id,"
"  descn,"
"  ground_altitude,"
"  surface_type,"
"  ambient_temperature,"
"  total_air_temperature,"
"  wind_velocity,"
"  wind_direction,"
"  precipitation_state,"
"  visibility,"
"  sea_level_pressure,"
"  pitot_icing,"
"  structural_icing,"
"  in_clouds"
") values ("
"  null,"
" :descn,"
" :ground_altitude,"
" :surface_type,"
" :ambient_temperature,"
" :total_air_temperature,"
" :wind_velocity,"
" :wind_direction,"
" :precipitation_state,"
" :visibility,"
" :sea_level_pressure,"
" :pitot_icing,"
" :structural_icing,"
" :in_clouds"
");");
        }
        if (selectQuery == nullptr) {
            selectQuery = std::make_unique<QSqlQuery>();
            selectQuery->prepare("select s.descn from scenario s where s.id = :id;");
        }
    }
};

// PUBLIC

SQLiteScenarioDao::SQLiteScenarioDao() noexcept
    : d(std::make_unique<SQLiteScenarioDaoPrivate>())
{
}

SQLiteScenarioDao::~SQLiteScenarioDao() noexcept
{}

bool SQLiteScenarioDao::addScenario(Scenario &scenario)  noexcept
{
    d->initQueries();
    const FlightCondition &flightCondition = scenario.getFlightConditionConst();
    d->insertQuery->bindValue(":descn", scenario.getDescription(), QSql::In);
    d->insertQuery->bindValue(":ground_altitude", flightCondition.groundAltitude, QSql::In);
    d->insertQuery->bindValue(":surface_type", Enum::toUnderlyingType(flightCondition.surfaceType), QSql::In);
    d->insertQuery->bindValue(":ambient_temperature", flightCondition.ambientTemperature, QSql::In);
    d->insertQuery->bindValue(":total_air_temperature", flightCondition.totalAirTemperature, QSql::In);
    d->insertQuery->bindValue(":wind_velocity", flightCondition.windVelocity, QSql::In);
    d->insertQuery->bindValue(":wind_direction", flightCondition.windDirection, QSql::In);
    d->insertQuery->bindValue(":precipitation_state", Enum::toUnderlyingType(flightCondition.precipitationState), QSql::In);
    d->insertQuery->bindValue(":visibility", flightCondition.visibility, QSql::In);
    d->insertQuery->bindValue(":sea_level_pressure", flightCondition.seaLevelPressure, QSql::In);
    d->insertQuery->bindValue(":pitot_icing", flightCondition.pitotIcingPercent, QSql::In);
    d->insertQuery->bindValue(":structural_icing", flightCondition.structuralIcingPercent, QSql::In);
    d->insertQuery->bindValue(":in_clouds", flightCondition.inClouds ? 1 : 0, QSql::In);
    bool ok = d->insertQuery->exec();
    if (ok) {
        qint64 id = d->insertQuery->lastInsertId().toLongLong(&ok);
        scenario.setId(id);
#ifdef DEBUG
    } else {
        qDebug("addScenario: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
#endif
    }
    if (ok) {
        // TODO IMPLEMENT ME!!
        //scenario.getFlightCondition()
    }
    return ok;
}

Scenario SQLiteScenarioDao::getScenario(qint64 id) const noexcept
{
    return Scenario();
}
