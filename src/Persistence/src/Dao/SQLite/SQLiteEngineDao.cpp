/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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

#include "../../../../Model/src/EngineData.h"
#include "../../ConnectionManager.h"
#include "SQLiteEngineDao.h"

// PUBLIC

SQLiteEngineDao::SQLiteEngineDao() noexcept
{}

SQLiteEngineDao::~SQLiteEngineDao() noexcept
{}

bool SQLiteEngineDao::add(std::int64_t aircraftId, const EngineData &data)  noexcept
{
    QSqlQuery query;
    query.prepare(
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
        "  general_engine_starter4,"
        "  general_engine_combustion1,"
        "  general_engine_combustion2,"
        "  general_engine_combustion3,"
        "  general_engine_combustion4"
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
        " :general_engine_starter4,"
        " :general_engine_combustion1,"
        " :general_engine_combustion2,"
        " :general_engine_combustion3,"
        " :general_engine_combustion4"
        ");"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    query.bindValue(":timestamp", QVariant::fromValue(data.timestamp));
    query.bindValue(":throttle_lever_position1", data.throttleLeverPosition1);
    query.bindValue(":throttle_lever_position2", data.throttleLeverPosition2);
    query.bindValue(":throttle_lever_position3", data.throttleLeverPosition3);
    query.bindValue(":throttle_lever_position4", data.throttleLeverPosition4);
    query.bindValue(":propeller_lever_position1", data.propellerLeverPosition1);
    query.bindValue(":propeller_lever_position2", data.propellerLeverPosition2);
    query.bindValue(":propeller_lever_position3", data.propellerLeverPosition3);
    query.bindValue(":propeller_lever_position4", data.propellerLeverPosition4);
    query.bindValue(":mixture_lever_position1", data.mixtureLeverPosition1);
    query.bindValue(":mixture_lever_position2", data.mixtureLeverPosition2);
    query.bindValue(":mixture_lever_position3", data.mixtureLeverPosition3);
    query.bindValue(":mixture_lever_position4", data.mixtureLeverPosition4);
    query.bindValue(":cowl_flap_position1", data.cowlFlapPosition1);
    query.bindValue(":cowl_flap_position2", data.cowlFlapPosition2);
    query.bindValue(":cowl_flap_position3", data.cowlFlapPosition3);
    query.bindValue(":cowl_flap_position4", data.cowlFlapPosition4);
    query.bindValue(":electrical_master_battery1", data.electricalMasterBattery1);
    query.bindValue(":electrical_master_battery2", data.electricalMasterBattery2);
    query.bindValue(":electrical_master_battery3", data.electricalMasterBattery3);
    query.bindValue(":electrical_master_battery4", data.electricalMasterBattery4);
    query.bindValue(":general_engine_starter1", data.generalEngineStarter1);
    query.bindValue(":general_engine_starter2", data.generalEngineStarter2);
    query.bindValue(":general_engine_starter3", data.generalEngineStarter3);
    query.bindValue(":general_engine_starter4", data.generalEngineStarter4);
    query.bindValue(":general_engine_combustion1", data.generalEngineCombustion1);
    query.bindValue(":general_engine_combustion2", data.generalEngineCombustion2);
    query.bindValue(":general_engine_combustion3", data.generalEngineCombustion3);
    query.bindValue(":general_engine_combustion4", data.generalEngineCombustion4);

    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteEngineDao::add: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteEngineDao::getByAircraftId(std::int64_t aircraftId, std::back_insert_iterator<std::vector<EngineData>> backInsertIterator) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   engine e "
        "where  e.aircraft_id = :aircraft_id "
        "order by e.timestamp asc;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
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
        const int generalEngineCombustion1Idx = record.indexOf("general_engine_combustion1");
        const int generalEngineCombustion2Idx = record.indexOf("general_engine_combustion2");
        const int generalEngineCombustion3Idx = record.indexOf("general_engine_combustion3");
        const int generalEngineCombustion4Idx = record.indexOf("general_engine_combustion4");

        while (query.next()) {

            EngineData data;

            data.timestamp = query.value(timestampIdx).toLongLong();
            data.throttleLeverPosition1 = query.value(throttleLeverPosition1Idx).toInt();
            data.throttleLeverPosition2 = query.value(throttleLeverPosition2Idx).toInt();
            data.throttleLeverPosition3 = query.value(throttleLeverPosition3Idx).toInt();
            data.throttleLeverPosition4 = query.value(throttleLeverPosition4Idx).toInt();
            data.propellerLeverPosition1 = query.value(propellerLeverPosition1Idx).toInt();
            data.propellerLeverPosition2 = query.value(propellerLeverPosition2Idx).toInt();
            data.propellerLeverPosition3 = query.value(propellerLeverPosition3Idx).toInt();
            data.propellerLeverPosition4 = query.value(propellerLeverPosition4Idx).toInt();
            data.mixtureLeverPosition1 = query.value(mixtureLeverPosition1Idx).toInt();
            data.mixtureLeverPosition2 = query.value(mixtureLeverPosition2Idx).toInt();
            data.mixtureLeverPosition3 = query.value(mixtureLeverPosition3Idx).toInt();
            data.mixtureLeverPosition4 = query.value(mixtureLeverPosition4Idx).toInt();
            data.cowlFlapPosition1 = query.value(cowlFlapPosition1Idx).toInt();
            data.cowlFlapPosition2 = query.value(cowlFlapPosition2Idx).toInt();
            data.cowlFlapPosition3 = query.value(cowlFlapPosition3Idx).toInt();
            data.cowlFlapPosition4 = query.value(cowlFlapPosition4Idx).toInt();
            data.electricalMasterBattery1 = query.value(electricalMasterBattery1Idx).toBool();
            data.electricalMasterBattery2 = query.value(electricalMasterBattery2Idx).toBool();
            data.electricalMasterBattery3 = query.value(electricalMasterBattery3Idx).toBool();
            data.electricalMasterBattery4 = query.value(electricalMasterBattery4Idx).toBool();
            data.generalEngineStarter1 = query.value(generalEngineStarter1Idx).toBool();
            data.generalEngineStarter2 = query.value(generalEngineStarter2Idx).toBool();
            data.generalEngineStarter3 = query.value(generalEngineStarter3Idx).toBool();
            data.generalEngineStarter4 = query.value(generalEngineStarter4Idx).toBool();
            data.generalEngineCombustion1 = query.value(generalEngineCombustion1Idx).toBool();
            data.generalEngineCombustion2 = query.value(generalEngineCombustion2Idx).toBool();
            data.generalEngineCombustion3 = query.value(generalEngineCombustion3Idx).toBool();
            data.generalEngineCombustion4 = query.value(generalEngineCombustion4Idx).toBool();

            backInsertIterator = std::move(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLiteEngineDao::getByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }
    return ok;
}

bool SQLiteEngineDao::deleteByFlightId(std::int64_t flightId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   engine "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    );

    query.bindValue(":flight_id", QVariant::fromValue(flightId));
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteEngineDao::deleteByFlightId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteEngineDao::deleteByAircraftId(std::int64_t aircraftId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   engine "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", QVariant::fromValue(aircraftId));
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLiteEngineDao::deleteByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return true;
}
