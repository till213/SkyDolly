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
#include <QSqlRecord>
#include <QVector>

#include "../../../../Kernel/src/Enum.h"
#include "../../Scenario.h"
#include "../../ScenarioDescription.h"
#include "../../FlightCondition.h"
#include "../../Dao/AircraftDaoIntf.h"
#include "../../Dao/DaoFactory.h"
#include "SQLiteScenarioDao.h"
#include "SQLiteScenarioDao.h"

class SQLiteScenarioDaoPrivate
{
public:
    SQLiteScenarioDaoPrivate() noexcept
        : insertQuery(nullptr),
          selectQuery(nullptr),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          aircraftDao(daoFactory->createAircraftDao())
    {
    }

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectQuery;
    std::unique_ptr<QSqlQuery> selectDescriptionsQuery;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<AircraftDaoIntf> aircraftDao;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();
            insertQuery->prepare(
"insert into scenario ("
"  id,"
"  description,"
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
" :description,"
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
            selectQuery->setForwardOnly(true);
            selectQuery->prepare(
"select * "
"from scenario s "
"where s.id = :id;");
        }
        if (selectDescriptionsQuery == nullptr) {
            selectDescriptionsQuery = std::make_unique<QSqlQuery>();
            selectDescriptionsQuery->setForwardOnly(true);
            selectDescriptionsQuery->prepare(
"select s.id, s.description, a.name "
"from   scenario s "
"join   aircraft a "
"where a.scenario_id = s.id;");
        }
    }
};

// PUBLIC

SQLiteScenarioDao::SQLiteScenarioDao() noexcept
    : d(std::make_unique<SQLiteScenarioDaoPrivate>())
{
}

SQLiteScenarioDao::~SQLiteScenarioDao() noexcept
{
}

bool SQLiteScenarioDao::addScenario(Scenario &scenario)  noexcept
{
    d->initQueries();
    const FlightCondition &flightCondition = scenario.getFlightConditionConst();
    d->insertQuery->bindValue(":description", scenario.getDescription(), QSql::In);
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
    d->insertQuery->bindValue(":in_clouds", flightCondition.inClouds, QSql::In);
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
        ok = d->aircraftDao->addAircraft(scenario.getId(), scenario.getUserAircraft());
    }
    return ok;
}

bool SQLiteScenarioDao::getScenario(qint64 id, Scenario &scenario) const noexcept
{
    d->initQueries();
    d->selectQuery->bindValue(":id", id);
    bool ok = d->selectQuery->exec();
    if (ok) {
        scenario.clear();
        int idFieldIndex = d->selectQuery->record().indexOf("id");
        int descriptionFieldIndex = d->selectQuery->record().indexOf("description");
        int groundAltitudeFieldIndex = d->selectQuery->record().indexOf("ground_altitude");
        int surfaceTypeFieldIndex = d->selectQuery->record().indexOf("surface_type");
        int ambientTemperatureFieldIndex = d->selectQuery->record().indexOf("ambient_temperature");
        int totalAirTemperatureFieldIndex = d->selectQuery->record().indexOf("total_air_temperature");
        int windVelocityFieldIndex = d->selectQuery->record().indexOf("wind_velocity");
        int windDirectionFieldIndex = d->selectQuery->record().indexOf("wind_direction");
        int precipitationStateFieldIndex = d->selectQuery->record().indexOf("precipitation_state");
        int visibilityFieldIndex = d->selectQuery->record().indexOf("visibility");
        int seaLevelPressureFieldIndex = d->selectQuery->record().indexOf("sea_level_pressure");
        int pitotIcingFieldIndex = d->selectQuery->record().indexOf("pitot_icing");
        int structuralIcingFieldIndex = d->selectQuery->record().indexOf("structural_icing");
        int inCloudsFieldIndex = d->selectQuery->record().indexOf("in_clouds");
        if (d->selectQuery->next()) {
            scenario.setId(d->selectQuery->value(idFieldIndex).toLongLong());
            scenario.setDescription(d->selectQuery->value(descriptionFieldIndex).toString());
            FlightCondition flightCondition;
            flightCondition.groundAltitude = d->selectQuery->value(groundAltitudeFieldIndex).toFloat();
            flightCondition.surfaceType = static_cast<SimType::SurfaceType>(d->selectQuery->value(surfaceTypeFieldIndex).toInt());
            flightCondition.ambientTemperature = d->selectQuery->value(ambientTemperatureFieldIndex).toFloat();
            flightCondition.totalAirTemperature = d->selectQuery->value(totalAirTemperatureFieldIndex).toFloat();
            flightCondition.windVelocity = d->selectQuery->value(windVelocityFieldIndex).toFloat();
            flightCondition.windDirection = d->selectQuery->value(windDirectionFieldIndex).toFloat();
            flightCondition.precipitationState = static_cast<SimType::PrecipitationState>(d->selectQuery->value(precipitationStateFieldIndex).toInt());
            flightCondition.visibility = d->selectQuery->value(visibilityFieldIndex).toFloat();
            flightCondition.seaLevelPressure = d->selectQuery->value(seaLevelPressureFieldIndex).toFloat();
            flightCondition.pitotIcingPercent = d->selectQuery->value(pitotIcingFieldIndex).toInt();
            flightCondition.structuralIcingPercent = d->selectQuery->value(structuralIcingFieldIndex).toInt();
            flightCondition.inClouds = d->selectQuery->value(inCloudsFieldIndex).toBool();

            scenario.setFlightCondition(flightCondition);
        }
    }
    return true;
}

QVector<ScenarioDescription> SQLiteScenarioDao::getScenarioDescriptions() const noexcept
{
    QVector<ScenarioDescription> descriptions;

    d->initQueries();
    bool ok = d->selectDescriptionsQuery->exec();
    if (ok) {
        int idFieldIndex = d->selectDescriptionsQuery->record().indexOf("id");
        int descriptionFieldIndex = d->selectDescriptionsQuery->record().indexOf("description");
        int aircraftNameFieldIndex = d->selectDescriptionsQuery->record().indexOf("name");
        while (d->selectDescriptionsQuery->next()) {
            ScenarioDescription description;
            description.id = d->selectDescriptionsQuery->value(idFieldIndex).toLongLong();
            description.description = d->selectDescriptionsQuery->value(descriptionFieldIndex).toString();
            description.aircraftName = d->selectDescriptionsQuery->value(aircraftNameFieldIndex).toString();
            descriptions.append(description);
        }
    }

    return descriptions;
}
