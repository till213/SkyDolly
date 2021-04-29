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
#include <QTimeZone>

#include "../../../../Kernel/src/Enum.h"
#include "../../../../Model/src/Aircraft.h"
#include "../../../../Model/src/AircraftInfo.h"
#include "../../../../Model/src/AircraftData.h"
#include "../../../../Model/src/Engine.h"
#include "../../../../Model/src/EngineData.h"
#include "../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../Model/src/AircraftHandle.h"
#include "../../../../Model/src/AircraftHandleData.h"
#include "../../../../Model/src/Light.h"
#include "../../../../Model/src/LightData.h"
#include "../../../../Model/src/FlightPlan.h"
#include "../../../../Model/src/FlightPlanData.h"
#include "../../Dao/ScenarioDaoIntf.h"
#include "../../Dao/AircraftDaoIntf.h"
#include "../../Dao/PositionDaoIntf.h"
#include "../../Dao/EngineDaoIntf.h"
#include "../../Dao/PrimaryFlightControlDaoIntf.h"
#include "../../Dao/SecondaryFlightControlDaoIntf.h"
#include "../../Dao/HandleDaoIntf.h"
#include "../../Dao/LightDaoIntf.h"
#include "../../Dao/FlightPlanDaoIntf.h"
#include "../../Dao/DaoFactory.h"
#include "../../ConnectionManager.h"
#include "SQLiteAircraftDao.h"

class SQLiteAircraftDaoPrivate
{
public:
    SQLiteAircraftDaoPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          positionDao(daoFactory->createPositionDao()),
          engineDao(daoFactory->createEngineDao()),
          primaryFlightControlDao(daoFactory->createPrimaryFlightControlDao()),
          secondaryFlightControlDao(daoFactory->createSecondaryFlightControlDao()),
          handleDao(daoFactory->createHandleDao()),
          lightDao(daoFactory->createLightDao()),
          flightPlanDao(daoFactory->createFlightPlanDao())
    {}

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByIdQuery;
    std::unique_ptr<QSqlQuery> selectByScenarioIdQuery;
    std::unique_ptr<QSqlQuery> deleteByScenarioIdQuery;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<PositionDaoIntf> positionDao;
    std::unique_ptr<EngineDaoIntf> engineDao;
    std::unique_ptr<PrimaryFlightControlDaoIntf> primaryFlightControlDao;
    std::unique_ptr<SecondaryFlightControlDaoIntf> secondaryFlightControlDao;
    std::unique_ptr<HandleDaoIntf> handleDao;
    std::unique_ptr<LightDaoIntf> lightDao;
    std::unique_ptr<FlightPlanDaoIntf> flightPlanDao;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();
            insertQuery->prepare(
"insert into aircraft ("
"  id,"
"  scenario_id,"
"  seq_nr,"
"  start_date,"
"  end_date,"
"  type,"
"  tail_number,"
"  airline,"
"  flight_number,"
"  category,"
"  initial_airspeed,"
"  wing_span,"
"  engine_type,"
"  nof_engines,"
"  altitude_above_ground,"
"  start_on_ground"
") values ("
"  null,"
" :scenario_id,"
" :seq_nr,"
" :start_date,"
" :end_date,"
" :type,"
" :tail_number,"
" :airline,"
" :flight_number,"
" :category,"
" :initial_airspeed,"
" :wing_span,"
" :engine_type,"
" :nof_engines,"
" :altitude_above_ground,"
" :start_on_ground"
");");
        }
        if (selectByIdQuery == nullptr) {
            selectByIdQuery = std::make_unique<QSqlQuery>();
            selectByIdQuery->setForwardOnly(true);
            selectByIdQuery->prepare(
"select * "
"from aircraft a "
"where a.id = :id;");
        }
        if (selectByScenarioIdQuery == nullptr) {
            selectByScenarioIdQuery = std::make_unique<QSqlQuery>();
            selectByScenarioIdQuery->setForwardOnly(true);
            selectByScenarioIdQuery->prepare(
"select * "
"from   aircraft a "
"where  a.scenario_id = :scenario_id"
"  and  a.seq_nr      = :seq_nr;");
        }
        if (deleteByScenarioIdQuery == nullptr) {
            deleteByScenarioIdQuery = std::make_unique<QSqlQuery>();
            deleteByScenarioIdQuery->prepare(
"delete "
"from   aircraft "
"where  scenario_id = :scenario_id;");
        }
    }

    void resetQueries() noexcept
    {
        insertQuery = nullptr;
        selectByIdQuery = nullptr;
        selectByScenarioIdQuery = nullptr;
        deleteByScenarioIdQuery = nullptr;
    }
};

// PUBLIC

SQLiteAircraftDao::SQLiteAircraftDao(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SQLiteAircraftDaoPrivate>())
{
    frenchConnection();
}

SQLiteAircraftDao::~SQLiteAircraftDao() noexcept
{}

bool SQLiteAircraftDao::add(qint64 scenarioId, int sequenceNumber, Aircraft &aircraft)  noexcept
{
    d->initQueries();
    const AircraftInfo &info = aircraft.getAircraftInfoConst();
    d->insertQuery->bindValue(":scenario_id", scenarioId);
    d->insertQuery->bindValue(":seq_nr", sequenceNumber);
    d->insertQuery->bindValue(":start_date", info.startDate.toUTC());
    d->insertQuery->bindValue(":end_date", info.endDate.toUTC());
    d->insertQuery->bindValue(":type", info.type);
    d->insertQuery->bindValue(":tail_number", info.tailNumber);
    d->insertQuery->bindValue(":airline", info.airline);
    d->insertQuery->bindValue(":flight_number", info.flightNumber);
    d->insertQuery->bindValue(":category", info.category);
    d->insertQuery->bindValue(":initial_airspeed", info.initialAirspeed);
    d->insertQuery->bindValue(":wing_span", info.wingSpan);
    d->insertQuery->bindValue(":engine_type", Enum::toUnderlyingType(info.engineType));
    d->insertQuery->bindValue(":nof_engines", info.numberOfEngines);
    d->insertQuery->bindValue(":altitude_above_ground", info.altitudeAboveGround);
    d->insertQuery->bindValue(":start_on_ground", info.startOnGround);

    bool ok = d->insertQuery->exec();
    if (ok) {
        qint64 id = d->insertQuery->lastInsertId().toLongLong(&ok);
        aircraft.setId(id);
#ifdef DEBUG
    } else {
        qDebug("SQLiteAircraftDao::add: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
#endif
    }
    if (ok) {
        for (const AircraftData &data : aircraft.getAllConst()) {
            ok = d->positionDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const EngineData &data : aircraft.getEngineConst().getAllConst()) {
            ok = d->engineDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const PrimaryFlightControlData &data : aircraft.getPrimaryFlightControlConst().getAllConst()) {
            ok = d->primaryFlightControlDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const SecondaryFlightControlData &data : aircraft.getSecondaryFlightControlConst().getAllConst()) {
            ok = d->secondaryFlightControlDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const AircraftHandleData &data : aircraft.getAircraftHandleConst().getAllConst()) {
            ok = d->handleDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        for (const LightData &data : aircraft.getLightConst().getAllConst()) {
            ok = d->lightDao->add(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    if (ok) {
        ok = d->flightPlanDao->add(aircraft.getId(), aircraft.getFlightPlanConst().getAllConst());
    }
    return ok;
}

bool SQLiteAircraftDao::getById(qint64 id, Aircraft &aircraft) const noexcept
{
    // TODO IMPLEMENT ME!!!
    return true;
}

bool SQLiteAircraftDao::getByScenarioId(qint64 scenarioId, int sequenceNumber, Aircraft &aircraft) const noexcept
{
    d->initQueries();
    d->selectByScenarioIdQuery->bindValue(":scenario_id", scenarioId);
    d->selectByScenarioIdQuery->bindValue(":seq_nr", sequenceNumber);
    bool ok = d->selectByScenarioIdQuery->exec();
    if (ok) {
        aircraft.clear();
        int idIdx = d->selectByScenarioIdQuery->record().indexOf("id");
        int startDateIdx = d->selectByScenarioIdQuery->record().indexOf("start_date");
        int endDateIdx = d->selectByScenarioIdQuery->record().indexOf("end_date");
        int typeIdx = d->selectByScenarioIdQuery->record().indexOf("type");
        int tailNumberIdx = d->selectByScenarioIdQuery->record().indexOf("tail_number");
        int airlineIdx = d->selectByScenarioIdQuery->record().indexOf("airline");
        int flightNumberIdx = d->selectByScenarioIdQuery->record().indexOf("flight_number");
        int categoryIdx = d->selectByScenarioIdQuery->record().indexOf("category");
        int initialAirspeedIdx = d->selectByScenarioIdQuery->record().indexOf("initial_airspeed");
        int wingSpanIdx = d->selectByScenarioIdQuery->record().indexOf("wing_span");
        int engineTypeIdx = d->selectByScenarioIdQuery->record().indexOf("engine_type");
        int nofEnginesIdx = d->selectByScenarioIdQuery->record().indexOf("nof_engines");
        int airCraftAltitudeAboveGroundIdx = d->selectByScenarioIdQuery->record().indexOf("altitude_above_ground");
        int startOnGroundIdx = d->selectByScenarioIdQuery->record().indexOf("start_on_ground");
        if (d->selectByScenarioIdQuery->next()) {
            aircraft.setId(d->selectByScenarioIdQuery->value(idIdx).toLongLong());

            AircraftInfo info;
            QDateTime dateTime = d->selectByScenarioIdQuery->value(startDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            info.startDate = dateTime.toLocalTime();
            dateTime = d->selectByScenarioIdQuery->value(endDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            info.endDate = dateTime.toLocalTime();
            info.type = d->selectByScenarioIdQuery->value(typeIdx).toString();
            info.tailNumber = d->selectByScenarioIdQuery->value(tailNumberIdx).toString();
            info.airline = d->selectByScenarioIdQuery->value(airlineIdx).toString();
            info.flightNumber = d->selectByScenarioIdQuery->value(flightNumberIdx).toString();
            info.category = d->selectByScenarioIdQuery->value(categoryIdx).toString();
            info.initialAirspeed = d->selectByScenarioIdQuery->value(initialAirspeedIdx).toInt();
            info.wingSpan = d->selectByScenarioIdQuery->value(wingSpanIdx).toInt();
            info.engineType = static_cast<SimType::EngineType>(d->selectByScenarioIdQuery->value(engineTypeIdx).toInt());
            info.numberOfEngines = d->selectByScenarioIdQuery->value(nofEnginesIdx).toInt();
            info.altitudeAboveGround = d->selectByScenarioIdQuery->value(airCraftAltitudeAboveGroundIdx).toInt();
            info.startOnGround = d->selectByScenarioIdQuery->value(startOnGroundIdx).toBool();

            aircraft.setAircraftInfo(info);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteAircraftDao::getByScenarioId: SQL error: %s", qPrintable(d->selectByScenarioIdQuery->lastError().databaseText() + " - error code: " + d->selectByScenarioIdQuery->lastError().nativeErrorCode()));
#endif
    }

    if (ok) {
        ok = d->positionDao->getByAircraftId(aircraft.getId(), aircraft.getAll());
    }
    if (ok) {
        ok = d->engineDao->getByAircraftId(aircraft.getId(), aircraft.getEngine().getAll());
    }
    if (ok) {
        ok = d->primaryFlightControlDao->getByAircraftId(aircraft.getId(), aircraft.getPrimaryFlightControl().getAll());
    }
    if (ok) {
        ok = d->secondaryFlightControlDao->getByAircraftId(aircraft.getId(), aircraft.getSecondaryFlightControl().getAll());
    }
    if (ok) {
        ok = d->handleDao->getByAircraftId(aircraft.getId(), aircraft.getAircraftHandle().getAll());
    }
    if (ok) {
        ok = d->lightDao->getByAircraftId(aircraft.getId(), aircraft.getLight().getAll());
    }
    if (ok) {
        ok = d->flightPlanDao->getByAircraftId(aircraft.getId(), aircraft.getFlightPlan().getAll());
    }

    return ok;
}

bool SQLiteAircraftDao::deleteByScenarioId(qint64 scenarioId) noexcept
{
    d->initQueries();
    // Delete "bottom-up" in order not to violate foreign key constraints
    bool ok = d->positionDao->deleteByScenarioId(scenarioId);
    if (ok) {
        ok = d->engineDao->deleteByScenarioId(scenarioId);
    }
    if (ok) {
        ok = d->primaryFlightControlDao->deleteByScenarioId(scenarioId);
    }
    if (ok) {
        ok = d->secondaryFlightControlDao->deleteByScenarioId(scenarioId);
    }
    if (ok) {
        ok = d->handleDao->deleteByScenarioId(scenarioId);
    }
    if (ok) {
        ok = d->lightDao->deleteByScenarioId(scenarioId);
    }
    if (ok) {
        ok = d->flightPlanDao->deleteByScenarioId(scenarioId);
    }
    if (ok) {

        d->deleteByScenarioIdQuery->bindValue(":scenario_id", scenarioId);
        ok = d->deleteByScenarioIdQuery->exec();
#ifdef DEBUG
        if (!ok) {
            qDebug("SQLiteAircraftDao::deleteByScenarioId: SQL error: %s", qPrintable(d->deleteByScenarioIdQuery->lastError().databaseText() + " - error code: " + d->deleteByScenarioIdQuery->lastError().nativeErrorCode()));
        }
#endif
    }

    return ok;
}

// PRIVATE

void SQLiteAircraftDao::frenchConnection() noexcept
{
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &SQLiteAircraftDao::handleConnectionChanged);
}

// PRIVATE SLOTS

void SQLiteAircraftDao::handleConnectionChanged() noexcept
{
    d->resetQueries();
}
