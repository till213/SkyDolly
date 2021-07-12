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

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>

#include "../../../../Kernel/src/Enum.h"
#include "../../../../Model/src/PrimaryFlightControlData.h"
#include "../../ConnectionManager.h"
#include "SQLitePrimaryFlightControlDao.h"

class SQLitePrimaryFlightControlDaoPrivate
{
public:
    SQLitePrimaryFlightControlDaoPrivate() noexcept
    {}

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByAircraftIdQuery;
    std::unique_ptr<QSqlQuery> deleteByFlightIdQuery;
    std::unique_ptr<QSqlQuery> deleteByIdQuery;

    void initQueries()
    {
        if (insertQuery == nullptr) {
            insertQuery = std::make_unique<QSqlQuery>();

        }
        if (selectByAircraftIdQuery == nullptr) {
            selectByAircraftIdQuery = std::make_unique<QSqlQuery>();

        }
        if (deleteByFlightIdQuery == nullptr) {
            deleteByFlightIdQuery = std::make_unique<QSqlQuery>();

        }
        if (deleteByIdQuery == nullptr) {
            deleteByIdQuery = std::make_unique<QSqlQuery>();

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

SQLitePrimaryFlightControlDao::SQLitePrimaryFlightControlDao() noexcept
{}

SQLitePrimaryFlightControlDao::~SQLitePrimaryFlightControlDao() noexcept
{}

bool SQLitePrimaryFlightControlDao::add(qint64 aircraftId, const PrimaryFlightControlData &primaryFlightControlData)  noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into primary_flight_control ("
        "  aircraft_id,"
        "  timestamp,"
        "  rudder_position,"
        "  elevator_position,"
        "  aileron_position"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :rudder_position,"
        " :elevator_position,"
        " :aileron_position"
        ");"
    );

    query.bindValue(":aircraft_id", aircraftId);
    query.bindValue(":timestamp", primaryFlightControlData.timestamp);
    query.bindValue(":rudder_position", primaryFlightControlData.rudderPosition);
    query.bindValue(":elevator_position", primaryFlightControlData.elevatorPosition);
    query.bindValue(":aileron_position", primaryFlightControlData.aileronPosition);

    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePrimaryFlightControlDao::add: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLitePrimaryFlightControlDao::getByAircraftId(qint64 aircraftId, std::insert_iterator<std::vector<PrimaryFlightControlData>> insertIterator) const noexcept
{
    QSqlQuery query;
    query.prepare(
        "select * "
        "from   primary_flight_control pfc "
        "where  pfc.aircraft_id = :aircraft_id "
        "order by pfc.timestamp asc;"
    );

    query.bindValue(":aircraft_id", aircraftId);
    bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int timestampIdx = record.indexOf("timestamp");
        const int rudderPositionIdx = record.indexOf("rudder_position");
        const int elevatorPositionIdx = record.indexOf("elevator_position");
        const int aileronPositionIdx = record.indexOf("aileron_position");
        while (query.next()) {

            PrimaryFlightControlData data;

            data.timestamp = query.value(timestampIdx).toLongLong();
            data.rudderPosition = query.value(rudderPositionIdx).toDouble();
            data.elevatorPosition = query.value(elevatorPositionIdx).toDouble();
            data.aileronPosition = query.value(aileronPositionIdx).toDouble();

            insertIterator = std::move(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLitePrimaryFlightControlDao::getByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }

    return ok;
}

bool SQLitePrimaryFlightControlDao::deleteByFlightId(qint64 flightId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   primary_flight_control "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    );

    query.bindValue(":flight_id", flightId);
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePrimaryFlightControlDao::deleteByFlightId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLitePrimaryFlightControlDao::deleteByAircraftId(qint64 aircraftId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   primary_flight_control "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", aircraftId);
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePrimaryFlightControlDao::deleteByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return true;
}
