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

#include <QObject>
#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>

#include "../../../../Model/src/EngineData.h"
#include "../../ConnectionManager.h"
#include "SQLiteEngineDao.h"

class SQLiteEngineDaoPrivate
{
public:
    SQLiteEngineDaoPrivate() noexcept
    {}

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByAircraftIdQuery;
    std::unique_ptr<QSqlQuery> deleteByFlightIdQuery;
    std::unique_ptr<QSqlQuery> deleteByIdQuery;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();
            insertQuery->prepare(
"insert into engine ("
"  aircraft_id,"
"  timestamp,"
"  throttle_lever_position1,"
"  throttle_lever_position2,"
"  throttle_lever_position3,"
"  throttle_lever_position4,"
"  propeller_lever_position1,"
"  propeller_lever_position2,"
"  propeller_lever_position3,"
"  propeller_lever_position4,"
"  mixture_lever_position1,"
"  mixture_lever_position2,"
"  mixture_lever_position3,"
"  mixture_lever_position4,"
"  cowl_flap_position1,"
"  cowl_flap_position2,"
"  cowl_flap_position3,"
"  cowl_flap_position4,"
"  electrical_master_battery1,"
"  electrical_master_battery2,"
"  electrical_master_battery3,"
"  electrical_master_battery4,"
"  general_engine_starter1,"
"  general_engine_starter2,"
"  general_engine_starter3,"
"  general_engine_starter4"
") values ("
" :aircraft_id,"
" :timestamp,"
" :throttle_lever_position1,"
" :throttle_lever_position2,"
" :throttle_lever_position3,"
" :throttle_lever_position4,"
" :propeller_lever_position1,"
" :propeller_lever_position2,"
" :propeller_lever_position3,"
" :propeller_lever_position4,"
" :mixture_lever_position1,"
" :mixture_lever_position2,"
" :mixture_lever_position3,"
" :mixture_lever_position4,"
" :cowl_flap_position1,"
" :cowl_flap_position2,"
" :cowl_flap_position3,"
" :cowl_flap_position4,"
" :electrical_master_battery1,"
" :electrical_master_battery2,"
" :electrical_master_battery3,"
" :electrical_master_battery4,"
" :general_engine_starter1,"
" :general_engine_starter2,"
" :general_engine_starter3,"
" :general_engine_starter4"
");");            
        }
        if (selectByAircraftIdQuery == nullptr) {
            selectByAircraftIdQuery = std::make_unique<QSqlQuery>();
            selectByAircraftIdQuery->prepare(
"select * "
"from   engine e "
"where  e.aircraft_id = :aircraft_id "
"order by e.timestamp asc;");
        }
        if (deleteByFlightIdQuery == nullptr) {
            deleteByFlightIdQuery = std::make_unique<QSqlQuery>();
            deleteByFlightIdQuery->prepare(
"delete "
"from   engine "
"where  aircraft_id in (select a.id "
"                       from aircraft a"
"                       where a.flight_id = :flight_id"
"                      );");
        }
        if (deleteByIdQuery == nullptr) {
            deleteByIdQuery = std::make_unique<QSqlQuery>();
            deleteByIdQuery->prepare(
"delete "
"from   engine "
"where  aircraft_id = :aircraft_id;");
        }
    }

    void resetQueries() noexcept
    {
        insertQuery = nullptr;
        selectByAircraftIdQuery = nullptr;
        deleteByFlightIdQuery = nullptr;
        deleteByIdQuery = nullptr;
    }
};

// PUBLIC

SQLiteEngineDao::SQLiteEngineDao(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SQLiteEngineDaoPrivate>())
{
    frenchConnection();
}

SQLiteEngineDao::~SQLiteEngineDao() noexcept
{}

bool SQLiteEngineDao::add(qint64 aircraftId, const EngineData &data)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId);
    d->insertQuery->bindValue(":timestamp", data.timestamp);
    d->insertQuery->bindValue(":throttle_lever_position1", data.throttleLeverPosition1);
    d->insertQuery->bindValue(":throttle_lever_position2", data.throttleLeverPosition2);
    d->insertQuery->bindValue(":throttle_lever_position3", data.throttleLeverPosition3);
    d->insertQuery->bindValue(":throttle_lever_position4", data.throttleLeverPosition4);
    d->insertQuery->bindValue(":propeller_lever_position1", data.propellerLeverPosition1);
    d->insertQuery->bindValue(":propeller_lever_position2", data.propellerLeverPosition2);
    d->insertQuery->bindValue(":propeller_lever_position3", data.propellerLeverPosition3);
    d->insertQuery->bindValue(":propeller_lever_position4", data.propellerLeverPosition4);
    d->insertQuery->bindValue(":mixture_lever_position1", data.mixtureLeverPosition1);
    d->insertQuery->bindValue(":mixture_lever_position2", data.mixtureLeverPosition2);
    d->insertQuery->bindValue(":mixture_lever_position3", data.mixtureLeverPosition3);
    d->insertQuery->bindValue(":mixture_lever_position4", data.mixtureLeverPosition4);
    d->insertQuery->bindValue(":cowl_flap_position1", data.cowlFlapPosition1);
    d->insertQuery->bindValue(":cowl_flap_position2", data.cowlFlapPosition2);
    d->insertQuery->bindValue(":cowl_flap_position3", data.cowlFlapPosition3);
    d->insertQuery->bindValue(":cowl_flap_position4", data.cowlFlapPosition4);
    d->insertQuery->bindValue(":electrical_master_battery1", data.electricalMasterBattery1);
    d->insertQuery->bindValue(":electrical_master_battery2", data.electricalMasterBattery2);
    d->insertQuery->bindValue(":electrical_master_battery3", data.electricalMasterBattery3);
    d->insertQuery->bindValue(":electrical_master_battery4", data.electricalMasterBattery4);
    d->insertQuery->bindValue(":general_engine_starter1", data.generalEngineStarter1);
    d->insertQuery->bindValue(":general_engine_starter2", data.generalEngineStarter2);
    d->insertQuery->bindValue(":general_engine_starter3", data.generalEngineStarter3);
    d->insertQuery->bindValue(":general_engine_starter4", data.generalEngineStarter4);

    bool ok = d->insertQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteEngineDao::add: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteEngineDao::getByAircraftId(qint64 aircraftId, std::insert_iterator<std::vector<EngineData>> insertIterator) const noexcept
{
    d->initQueries();
    d->selectByAircraftIdQuery->bindValue(":aircraft_id", aircraftId);
    bool ok = d->selectByAircraftIdQuery->exec();
    if (ok) {
        QSqlRecord record = d->selectByAircraftIdQuery->record();
        const int timestampIdx = record.indexOf("timestamp");
        const int throttleLeverPosition1Idx = record.indexOf("throttle_lever_position1");
        const int throttleLeverPosition2Idx = record.indexOf("throttle_lever_position2");
        const int throttleLeverPosition3Idx = record.indexOf("throttle_lever_position3");
        const int throttleLeverPosition4Idx = record.indexOf("throttle_lever_position4");
        const int propellerLeverPosition1Idx = record.indexOf("propeller_lever_position1");
        const int propellerLeverPosition2Idx = record.indexOf("propeller_lever_position2");
        const int propellerLeverPosition3Idx = record.indexOf("propeller_lever_position3");
        const int propellerLeverPosition4Idx = record.indexOf("propeller_lever_position4");
        const int mixtureLeverPosition1Idx = record.indexOf("mixture_lever_position1");
        const int mixtureLeverPosition2Idx = record.indexOf("mixture_lever_position2");
        const int mixtureLeverPosition3Idx = record.indexOf("mixture_lever_position3");
        const int mixtureLeverPosition4Idx = record.indexOf("mixture_lever_position4");
        const int cowlFlapPosition1Idx = record.indexOf("cowl_flap_position1");
        const int cowlFlapPosition2Idx = record.indexOf("cowl_flap_position2");
        const int cowlFlapPosition3Idx = record.indexOf("cowl_flap_position3");
        const int cowlFlapPosition4Idx = record.indexOf("cowl_flap_position4");
        const int electricalMasterBattery1Idx = record.indexOf("electrical_master_battery1");
        const int electricalMasterBattery2Idx = record.indexOf("electrical_master_battery2");
        const int electricalMasterBattery3Idx = record.indexOf("electrical_master_battery3");
        const int electricalMasterBattery4Idx = record.indexOf("electrical_master_battery4");
        const int generalEngineStarter1Idx = record.indexOf("general_engine_starter1");
        const int generalEngineStarter2Idx = record.indexOf("general_engine_starter2");
        const int generalEngineStarter3Idx = record.indexOf("general_engine_starter3");
        const int generalEngineStarter4Idx = record.indexOf("general_engine_starter4");

        while (d->selectByAircraftIdQuery->next()) {

            EngineData data;

            data.timestamp = d->selectByAircraftIdQuery->value(timestampIdx).toLongLong();
            data.throttleLeverPosition1 = d->selectByAircraftIdQuery->value(throttleLeverPosition1Idx).toInt();
            data.throttleLeverPosition2 = d->selectByAircraftIdQuery->value(throttleLeverPosition2Idx).toInt();
            data.throttleLeverPosition3 = d->selectByAircraftIdQuery->value(throttleLeverPosition3Idx).toInt();
            data.throttleLeverPosition4 = d->selectByAircraftIdQuery->value(throttleLeverPosition4Idx).toInt();
            data.propellerLeverPosition1 = d->selectByAircraftIdQuery->value(propellerLeverPosition1Idx).toInt();
            data.propellerLeverPosition2 = d->selectByAircraftIdQuery->value(propellerLeverPosition2Idx).toInt();
            data.propellerLeverPosition3 = d->selectByAircraftIdQuery->value(propellerLeverPosition3Idx).toInt();
            data.propellerLeverPosition4 = d->selectByAircraftIdQuery->value(propellerLeverPosition4Idx).toInt();
            data.mixtureLeverPosition1 = d->selectByAircraftIdQuery->value(mixtureLeverPosition1Idx).toInt();
            data.mixtureLeverPosition2 = d->selectByAircraftIdQuery->value(mixtureLeverPosition2Idx).toInt();
            data.mixtureLeverPosition3 = d->selectByAircraftIdQuery->value(mixtureLeverPosition3Idx).toInt();
            data.mixtureLeverPosition4 = d->selectByAircraftIdQuery->value(mixtureLeverPosition4Idx).toInt();
            data.cowlFlapPosition1 = d->selectByAircraftIdQuery->value(cowlFlapPosition1Idx).toInt();
            data.cowlFlapPosition2 = d->selectByAircraftIdQuery->value(cowlFlapPosition2Idx).toInt();
            data.cowlFlapPosition3 = d->selectByAircraftIdQuery->value(cowlFlapPosition3Idx).toInt();
            data.cowlFlapPosition4 = d->selectByAircraftIdQuery->value(cowlFlapPosition4Idx).toInt();
            data.electricalMasterBattery1 = d->selectByAircraftIdQuery->value(electricalMasterBattery1Idx).toBool();
            data.electricalMasterBattery2 = d->selectByAircraftIdQuery->value(electricalMasterBattery2Idx).toBool();
            data.electricalMasterBattery3 = d->selectByAircraftIdQuery->value(electricalMasterBattery3Idx).toBool();
            data.electricalMasterBattery4 = d->selectByAircraftIdQuery->value(electricalMasterBattery4Idx).toBool();
            data.generalEngineStarter1 = d->selectByAircraftIdQuery->value(generalEngineStarter1Idx).toBool();
            data.generalEngineStarter2 = d->selectByAircraftIdQuery->value(generalEngineStarter2Idx).toBool();
            data.generalEngineStarter3 = d->selectByAircraftIdQuery->value(generalEngineStarter3Idx).toBool();
            data.generalEngineStarter4 = d->selectByAircraftIdQuery->value(generalEngineStarter4Idx).toBool();

            insertIterator = std::move(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteEngineDao::getByAircraftId: SQL error: %s", qPrintable(d->selectByAircraftIdQuery->lastError().databaseText() + " - error code: " + d->selectByAircraftIdQuery->lastError().nativeErrorCode()));
#endif
    }
    return ok;
}

bool SQLiteEngineDao::deleteByFlightId(qint64 flightId) noexcept
{
    d->initQueries();
    d->deleteByFlightIdQuery->bindValue(":flight_id", flightId);
    bool ok = d->deleteByFlightIdQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteEngineDao::deleteByFlightId: SQL error: %s", qPrintable(d->deleteByFlightIdQuery->lastError().databaseText() + " - error code: " + d->deleteByFlightIdQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteEngineDao::deleteByAircraftId(qint64 aircraftId) noexcept
{
    d->initQueries();
    d->deleteByIdQuery->bindValue(":aircraft_id", aircraftId);
    bool ok = d->deleteByIdQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteEngineDao::deleteByAircraftId: SQL error: %s", qPrintable(d->deleteByIdQuery->lastError().databaseText() + " - error code: " + d->deleteByIdQuery->lastError().nativeErrorCode()));
    }
#endif
    return true;
}

// PRIVATE

void SQLiteEngineDao::frenchConnection() noexcept
{
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &SQLiteEngineDao::handleConnectionChanged);
}

// PRIVATE SLOTS

void SQLiteEngineDao::handleConnectionChanged() noexcept
{
    d->resetQueries();
}

