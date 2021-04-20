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

#include "../../AircraftData.h"
#include "SQLiteEngineDao.h"

class SQLiteEngineDaoPrivate
{
public:
    SQLiteEngineDaoPrivate() noexcept
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
"insert into engine ("
"  aircraft_id,"
"  timestamp,"
"  throttle_level_position1,"
"  throttle_level_position2,"
"  throttle_level_position3,"
"  throttle_level_position4,"
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
" :throttle_level_position1,"
" :throttle_level_position2,"
" :throttle_level_position3,"
" :throttle_level_position4,"
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
        if (selectQuery == nullptr) {
            selectQuery = std::make_unique<QSqlQuery>();
            selectQuery->prepare("select a.name from engine a where a.id = :id;");
        }
    }
};

// PUBLIC

SQLiteEngineDao::SQLiteEngineDao() noexcept
    : d(std::make_unique<SQLiteEngineDaoPrivate>())
{
}

SQLiteEngineDao::~SQLiteEngineDao() noexcept
{}

bool SQLiteEngineDao::addEngine(qint64 aircraftId, const EngineData &engine)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId, QSql::In);
    d->insertQuery->bindValue(":timestamp", engine.timestamp, QSql::In);
    d->insertQuery->bindValue(":throttle_level_position1", engine.throttleLeverPosition1, QSql::In);
    d->insertQuery->bindValue(":throttle_level_position2", engine.throttleLeverPosition2, QSql::In);
    d->insertQuery->bindValue(":throttle_level_position3", engine.throttleLeverPosition3, QSql::In);
    d->insertQuery->bindValue(":throttle_level_position4", engine.throttleLeverPosition4, QSql::In);
    d->insertQuery->bindValue(":propeller_lever_position1", engine.propellerLeverPosition1, QSql::In);
    d->insertQuery->bindValue(":propeller_lever_position2", engine.propellerLeverPosition2, QSql::In);
    d->insertQuery->bindValue(":propeller_lever_position3", engine.propellerLeverPosition3, QSql::In);
    d->insertQuery->bindValue(":propeller_lever_position4", engine.propellerLeverPosition4, QSql::In);
    d->insertQuery->bindValue(":mixture_lever_position1", engine.mixtureLeverPosition1, QSql::In);
    d->insertQuery->bindValue(":mixture_lever_position2", engine.mixtureLeverPosition2, QSql::In);
    d->insertQuery->bindValue(":mixture_lever_position3", engine.mixtureLeverPosition3, QSql::In);
    d->insertQuery->bindValue(":mixture_lever_position4", engine.mixtureLeverPosition4, QSql::In);
    d->insertQuery->bindValue(":cowl_flap_position1", engine.cowlFlapPosition1, QSql::In);
    d->insertQuery->bindValue(":cowl_flap_position2", engine.cowlFlapPosition2, QSql::In);
    d->insertQuery->bindValue(":cowl_flap_position3", engine.cowlFlapPosition3, QSql::In);
    d->insertQuery->bindValue(":cowl_flap_position4", engine.cowlFlapPosition4, QSql::In);
    d->insertQuery->bindValue(":electrical_master_battery1", engine.electricalMasterBattery1, QSql::In);
    d->insertQuery->bindValue(":electrical_master_battery2", engine.electricalMasterBattery2, QSql::In);
    d->insertQuery->bindValue(":electrical_master_battery3", engine.electricalMasterBattery3, QSql::In);
    d->insertQuery->bindValue(":electrical_master_battery4", engine.electricalMasterBattery4, QSql::In);
    d->insertQuery->bindValue(":general_engine_starter1", engine.generalEngineStarter1, QSql::In);
    d->insertQuery->bindValue(":general_engine_starter2", engine.generalEngineStarter2, QSql::In);
    d->insertQuery->bindValue(":general_engine_starter3", engine.generalEngineStarter3, QSql::In);
    d->insertQuery->bindValue(":general_engine_starter4", engine.generalEngineStarter4, QSql::In);

    bool ok = d->insertQuery->exec();

#ifdef DEBUG
    if (!ok) {
        qDebug("addEngine: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

EngineData SQLiteEngineDao::getEngine(qint64 aircraftId, qint64 timestamp) const noexcept
{
    return EngineData();
}
