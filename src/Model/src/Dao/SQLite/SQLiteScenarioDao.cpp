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

namespace  {
    constexpr int UserAircraftSequenceNumber = 1;
}

class SQLiteScenarioDaoPrivate
{
public:
    SQLiteScenarioDaoPrivate() noexcept
        : insertQuery(nullptr),
          selectByIdQuery(nullptr),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          aircraftDao(daoFactory->createAircraftDao())
    {
    }

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByIdQuery;
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
"  surface_type,"
"  ground_altitude,"
"  ambient_temperature,"
"  total_air_temperature,"
"  wind_velocity,"
"  wind_direction,"
"  visibility,"
"  sea_level_pressure,"
"  pitot_icing,"
"  structural_icing,"
"  precipitation_state,"
"  in_clouds"
") values ("
"  null,"
" :description,"
" :surface_type,"
" :ground_altitude,"
" :ambient_temperature,"
" :total_air_temperature,"
" :wind_velocity,"
" :wind_direction,"
" :visibility,"
" :sea_level_pressure,"
" :pitot_icing,"
" :structural_icing,"
" :precipitation_state,"
" :in_clouds"
");");
        }
        if (selectByIdQuery == nullptr) {
            selectByIdQuery = std::make_unique<QSqlQuery>();
            selectByIdQuery->setForwardOnly(true);
            selectByIdQuery->prepare(
"select * "
"from scenario s "
"where s.id = :id;");
        }
        if (selectDescriptionsQuery == nullptr) {
            selectDescriptionsQuery = std::make_unique<QSqlQuery>();
            selectDescriptionsQuery->setForwardOnly(true);
            selectDescriptionsQuery->prepare(
"select s.id, s.description, a.type "
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
    d->insertQuery->bindValue(":surface_type", Enum::toUnderlyingType(flightCondition.surfaceType), QSql::In);
    d->insertQuery->bindValue(":ground_altitude", flightCondition.groundAltitude, QSql::In);
    d->insertQuery->bindValue(":ambient_temperature", flightCondition.ambientTemperature, QSql::In);
    d->insertQuery->bindValue(":total_air_temperature", flightCondition.totalAirTemperature, QSql::In);
    d->insertQuery->bindValue(":wind_velocity", flightCondition.windVelocity, QSql::In);
    d->insertQuery->bindValue(":wind_direction", flightCondition.windDirection, QSql::In);
    d->insertQuery->bindValue(":visibility", flightCondition.visibility, QSql::In);
    d->insertQuery->bindValue(":sea_level_pressure", flightCondition.seaLevelPressure, QSql::In);
    d->insertQuery->bindValue(":pitot_icing", flightCondition.pitotIcingPercent, QSql::In);
    d->insertQuery->bindValue(":structural_icing", flightCondition.structuralIcingPercent, QSql::In);
    d->insertQuery->bindValue(":precipitation_state", Enum::toUnderlyingType(flightCondition.precipitationState), QSql::In);
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
        ok = d->aircraftDao->add(scenario.getId(), UserAircraftSequenceNumber, scenario.getUserAircraft());
    }
    return ok;
}

bool SQLiteScenarioDao::getScenarioById(qint64 id, Scenario &scenario) const noexcept
{
    d->initQueries();
    d->selectByIdQuery->bindValue(":id", id);
    bool ok = d->selectByIdQuery->exec();
    if (ok) {
        scenario.clear();
        int idIdx = d->selectByIdQuery->record().indexOf("id");
        int descriptionIdx = d->selectByIdQuery->record().indexOf("description");
        int surfaceTypeIdx = d->selectByIdQuery->record().indexOf("surface_type");
        int groundAltitudeIdx = d->selectByIdQuery->record().indexOf("ground_altitude");
        int ambientTemperatureIdx = d->selectByIdQuery->record().indexOf("ambient_temperature");
        int totalAirTemperatureIdx = d->selectByIdQuery->record().indexOf("total_air_temperature");
        int windVelocityIdx = d->selectByIdQuery->record().indexOf("wind_velocity");
        int windDirectionIdx = d->selectByIdQuery->record().indexOf("wind_direction");
        int visibilityIdx = d->selectByIdQuery->record().indexOf("visibility");
        int seaLevelPressureIdx = d->selectByIdQuery->record().indexOf("sea_level_pressure");
        int pitotIcingIdx = d->selectByIdQuery->record().indexOf("pitot_icing");
        int structuralIcingIdx = d->selectByIdQuery->record().indexOf("structural_icing");
        int precipitationStateIdx = d->selectByIdQuery->record().indexOf("precipitation_state");
        int inCloudsIdx = d->selectByIdQuery->record().indexOf("in_clouds");
        if (d->selectByIdQuery->next()) {
            scenario.setId(d->selectByIdQuery->value(idIdx).toLongLong());
            scenario.setDescription(d->selectByIdQuery->value(descriptionIdx).toString());
            FlightCondition flightCondition;
            flightCondition.surfaceType = static_cast<SimType::SurfaceType>(d->selectByIdQuery->value(surfaceTypeIdx).toInt());
            flightCondition.groundAltitude = d->selectByIdQuery->value(groundAltitudeIdx).toFloat();
            flightCondition.ambientTemperature = d->selectByIdQuery->value(ambientTemperatureIdx).toFloat();
            flightCondition.totalAirTemperature = d->selectByIdQuery->value(totalAirTemperatureIdx).toFloat();
            flightCondition.windVelocity = d->selectByIdQuery->value(windVelocityIdx).toFloat();
            flightCondition.windDirection = d->selectByIdQuery->value(windDirectionIdx).toFloat();
            flightCondition.visibility = d->selectByIdQuery->value(visibilityIdx).toFloat();
            flightCondition.seaLevelPressure = d->selectByIdQuery->value(seaLevelPressureIdx).toFloat();
            flightCondition.pitotIcingPercent = d->selectByIdQuery->value(pitotIcingIdx).toInt();
            flightCondition.structuralIcingPercent = d->selectByIdQuery->value(structuralIcingIdx).toInt();
            flightCondition.precipitationState = static_cast<SimType::PrecipitationState>(d->selectByIdQuery->value(precipitationStateIdx).toInt());
            flightCondition.inClouds = d->selectByIdQuery->value(inCloudsIdx).toBool();

            scenario.setFlightCondition(flightCondition);
        }
        Aircraft &userAircraft = scenario.getUserAircraft();
        ok = d->aircraftDao->getByScenarioId(id, UserAircraftSequenceNumber, userAircraft);
#ifdef DEBUG
    } else {
        qDebug("getScenarioById: SQL error: %s", qPrintable(d->selectByIdQuery->lastError().databaseText() + " - error code: " + d->selectByIdQuery->lastError().nativeErrorCode()));
#endif
    }
    return ok;
}

QVector<ScenarioDescription> SQLiteScenarioDao::getScenarioDescriptions() const noexcept
{
    QVector<ScenarioDescription> descriptions;

    d->initQueries();
    bool ok = d->selectDescriptionsQuery->exec();
    if (ok) {
        int idIdx = d->selectDescriptionsQuery->record().indexOf("id");
        int descriptionIdx = d->selectDescriptionsQuery->record().indexOf("description");
        int aircraftTypeIdx = d->selectDescriptionsQuery->record().indexOf("type");
        while (d->selectDescriptionsQuery->next()) {
            ScenarioDescription description;
            description.id = d->selectDescriptionsQuery->value(idIdx).toLongLong();
            description.description = d->selectDescriptionsQuery->value(descriptionIdx).toString();
            description.aircraftType = d->selectDescriptionsQuery->value(aircraftTypeIdx).toString();
            descriptions.append(description);
        }
#ifdef DEBUG
    } else {
        qDebug("getScenarioDescriptions: SQL error: %s", qPrintable(d->selectDescriptionsQuery->lastError().databaseText() + " - error code: " + d->selectDescriptionsQuery->lastError().nativeErrorCode()));
#endif
    }

    return descriptions;
}
