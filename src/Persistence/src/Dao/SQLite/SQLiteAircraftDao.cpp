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
#include "../../../../Model/src/Waypoint.h"
#include "../../Dao/FlightDaoIntf.h"
#include "../../Dao/AircraftDaoIntf.h"
#include "../../Dao/PositionDaoIntf.h"
#include "../../Dao/EngineDaoIntf.h"
#include "../../Dao/PrimaryFlightControlDaoIntf.h"
#include "../../Dao/SecondaryFlightControlDaoIntf.h"
#include "../../Dao/HandleDaoIntf.h"
#include "../../Dao/LightDaoIntf.h"
#include "../../Dao/WaypointDaoIntf.h"
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
    std::unique_ptr<QSqlQuery> selectByFlightIdQuery;
    std::unique_ptr<QSqlQuery> deleteByFlightIdQuery;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<PositionDaoIntf> positionDao;
    std::unique_ptr<EngineDaoIntf> engineDao;
    std::unique_ptr<PrimaryFlightControlDaoIntf> primaryFlightControlDao;
    std::unique_ptr<SecondaryFlightControlDaoIntf> secondaryFlightControlDao;
    std::unique_ptr<HandleDaoIntf> handleDao;
    std::unique_ptr<LightDaoIntf> lightDao;
    std::unique_ptr<WaypointDaoIntf> flightPlanDao;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();
            insertQuery->prepare(
"insert into aircraft ("
"  id,"
"  flight_id,"
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
" :flight_id,"
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
        if (selectByFlightIdQuery == nullptr) {
            selectByFlightIdQuery = std::make_unique<QSqlQuery>();
            selectByFlightIdQuery->setForwardOnly(true);
            selectByFlightIdQuery->prepare(
"select * "
"from   aircraft a "
"where  a.flight_id = :flight_id"
"  and  a.seq_nr      = :seq_nr;");
        }
        if (deleteByFlightIdQuery == nullptr) {
            deleteByFlightIdQuery = std::make_unique<QSqlQuery>();
            deleteByFlightIdQuery->prepare(
"delete "
"from   aircraft "
"where  flight_id = :flight_id;");
        }
    }

    void resetQueries() noexcept
    {
        insertQuery = nullptr;
        selectByIdQuery = nullptr;
        selectByFlightIdQuery = nullptr;
        deleteByFlightIdQuery = nullptr;
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

bool SQLiteAircraftDao::add(qint64 flightId, int sequenceNumber, Aircraft &aircraft)  noexcept
{
    d->initQueries();
    const AircraftInfo &info = aircraft.getAircraftInfoConst();
    d->insertQuery->bindValue(":flight_id", flightId);
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

bool SQLiteAircraftDao::getByFlightId(qint64 flightId, int sequenceNumber, Aircraft &aircraft) const noexcept
{
    d->initQueries();
    d->selectByFlightIdQuery->bindValue(":flight_id", flightId);
    d->selectByFlightIdQuery->bindValue(":seq_nr", sequenceNumber);
    bool ok = d->selectByFlightIdQuery->exec();
    if (ok) {
        aircraft.clear();
        const int idIdx = d->selectByFlightIdQuery->record().indexOf("id");
        const int startDateIdx = d->selectByFlightIdQuery->record().indexOf("start_date");
        const int endDateIdx = d->selectByFlightIdQuery->record().indexOf("end_date");
        const int typeIdx = d->selectByFlightIdQuery->record().indexOf("type");
        const int tailNumberIdx = d->selectByFlightIdQuery->record().indexOf("tail_number");
        const int airlineIdx = d->selectByFlightIdQuery->record().indexOf("airline");
        const int flightNumberIdx = d->selectByFlightIdQuery->record().indexOf("flight_number");
        const int categoryIdx = d->selectByFlightIdQuery->record().indexOf("category");
        const int initialAirspeedIdx = d->selectByFlightIdQuery->record().indexOf("initial_airspeed");
        const int wingSpanIdx = d->selectByFlightIdQuery->record().indexOf("wing_span");
        const int engineTypeIdx = d->selectByFlightIdQuery->record().indexOf("engine_type");
        const int nofEnginesIdx = d->selectByFlightIdQuery->record().indexOf("nof_engines");
        const int airCraftAltitudeAboveGroundIdx = d->selectByFlightIdQuery->record().indexOf("altitude_above_ground");
        const int startOnGroundIdx = d->selectByFlightIdQuery->record().indexOf("start_on_ground");
        if (d->selectByFlightIdQuery->next()) {
            aircraft.setId(d->selectByFlightIdQuery->value(idIdx).toLongLong());

            AircraftInfo info;
            QDateTime dateTime = d->selectByFlightIdQuery->value(startDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            info.startDate = dateTime.toLocalTime();
            dateTime = d->selectByFlightIdQuery->value(endDateIdx).toDateTime();
            dateTime.setTimeZone(QTimeZone::utc());
            info.endDate = dateTime.toLocalTime();
            info.type = d->selectByFlightIdQuery->value(typeIdx).toString();
            info.tailNumber = d->selectByFlightIdQuery->value(tailNumberIdx).toString();
            info.airline = d->selectByFlightIdQuery->value(airlineIdx).toString();
            info.flightNumber = d->selectByFlightIdQuery->value(flightNumberIdx).toString();
            info.category = d->selectByFlightIdQuery->value(categoryIdx).toString();
            info.initialAirspeed = d->selectByFlightIdQuery->value(initialAirspeedIdx).toInt();
            info.wingSpan = d->selectByFlightIdQuery->value(wingSpanIdx).toInt();
            info.engineType = static_cast<SimType::EngineType>(d->selectByFlightIdQuery->value(engineTypeIdx).toInt());
            info.numberOfEngines = d->selectByFlightIdQuery->value(nofEnginesIdx).toInt();
            info.altitudeAboveGround = d->selectByFlightIdQuery->value(airCraftAltitudeAboveGroundIdx).toInt();
            info.startOnGround = d->selectByFlightIdQuery->value(startOnGroundIdx).toBool();

            aircraft.setAircraftInfo(info);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteAircraftDao::getByFlightId: SQL error: %s", qPrintable(d->selectByFlightIdQuery->lastError().databaseText() + " - error code: " + d->selectByFlightIdQuery->lastError().nativeErrorCode()));
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
        ok = d->flightPlanDao->getByAircraftId(aircraft.getId(), aircraft.getFlightPlan());
    }

    emit aircraft.dataChanged();

    return ok;
}

bool SQLiteAircraftDao::deleteByFlightId(qint64 flightId) noexcept
{
    d->initQueries();
    // Delete "bottom-up" in order not to violate foreign key constraints
    bool ok = d->positionDao->deleteByFlightId(flightId);
    if (ok) {
        ok = d->engineDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->primaryFlightControlDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->secondaryFlightControlDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->handleDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->lightDao->deleteByFlightId(flightId);
    }
    if (ok) {
        ok = d->flightPlanDao->deleteByFlightId(flightId);
    }
    if (ok) {
        d->deleteByFlightIdQuery->bindValue(":flight_id", flightId);
        ok = d->deleteByFlightIdQuery->exec();
#ifdef DEBUG
        if (!ok) {
            qDebug("SQLiteAircraftDao::deleteByFlightId: SQL error: %s", qPrintable(d->deleteByFlightIdQuery->lastError().databaseText() + " - error code: " + d->deleteByFlightIdQuery->lastError().nativeErrorCode()));
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
