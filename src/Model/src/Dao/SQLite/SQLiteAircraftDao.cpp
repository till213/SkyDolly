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
#include "../../Aircraft.h"
#include "../../AircraftData.h"
#include "../../Dao/PositionDaoIntf.h"
#include "../../Dao/DaoFactory.h"
#include "SQLiteAircraftDao.h"

class SQLiteAircraftDaoPrivate
{
public:
    SQLiteAircraftDaoPrivate() noexcept
        : insertQuery(nullptr),
          selectQuery(nullptr),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          positionDao(daoFactory->createPositionDao())
    {
    }

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectQuery;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<PositionDaoIntf> positionDao;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();
            insertQuery->prepare(
"insert into aircraft ("
"  id,"
"  scenario_id,"
"  name,"
"  tail_number,"
"  airline,"
"  flight_number,"
"  category,"
"  initial_airspeed,"
"  wing_span,"
"  engine_type,"
"  nof_engines,"
"  altitude_above_ground"
") values ("
"  null,"
" :scenario_id,"
" :name,"
" :tail_number,"
" :airline,"
" :flight_number,"
" :category,"
" :initial_airspeed,"
" :wing_span,"
" :engine_type,"
" :nof_engines,"
" :altitude_above_ground"
");");
        }
        if (selectQuery == nullptr) {
            selectQuery = std::make_unique<QSqlQuery>();
            selectQuery->prepare("select a.name from aircraft a where a.id = :id;");
        }
    }
};

// PUBLIC

SQLiteAircraftDao::SQLiteAircraftDao() noexcept
    : d(std::make_unique<SQLiteAircraftDaoPrivate>())
{
}

SQLiteAircraftDao::~SQLiteAircraftDao() noexcept
{}

bool SQLiteAircraftDao::addAircraft(qint64 scenarioId, Aircraft &aircraft)  noexcept
{
    d->initQueries();
    const AircraftInfo &info = aircraft.getAircraftInfoConst();
    d->insertQuery->bindValue(":scenario_id", scenarioId, QSql::In);
    d->insertQuery->bindValue(":name", info.name, QSql::In);
    d->insertQuery->bindValue(":tail_number", info.atcId, QSql::In);
    d->insertQuery->bindValue(":airline", info.atcAirline, QSql::In);
    d->insertQuery->bindValue(":flight_number", info.atcFlightNumber, QSql::In);
    d->insertQuery->bindValue(":category", info.category, QSql::In);
    d->insertQuery->bindValue(":initial_airspeed", info.initialAirspeed, QSql::In);
    d->insertQuery->bindValue(":wing_span", info.wingSpan, QSql::In);
    d->insertQuery->bindValue(":engine_type", Enum::toUnderlyingType(info.engineType), QSql::In);
    d->insertQuery->bindValue(":nof_engines", info.numberOfEngines, QSql::In);
    d->insertQuery->bindValue(":altitude_above_ground", info.aircraftAltitudeAboveGround, QSql::In);

    bool ok = d->insertQuery->exec();
    if (ok) {
        qint64 id = d->insertQuery->lastInsertId().toLongLong(&ok);
        aircraft.setId(id);
#ifdef DEBUG
    } else {
        qDebug("addAircraft: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
#endif
    }
    if (ok) {
        for (const AircraftData &data : aircraft.getAll()) {
            ok = d->positionDao->addPosition(aircraft.getId(), data);
            if (!ok) {
                break;
            }
        }
    }
    return ok;
}

Aircraft SQLiteAircraftDao::getAircraft(qint64 id) const noexcept
{
    return Aircraft();
}
