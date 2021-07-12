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
#include "../../../../Model/src/PositionData.h"
#include "../../ConnectionManager.h"
#include "SQLitePositionDao.h"

class SQLitePositionDaoPrivate
{
public:
    SQLitePositionDaoPrivate() noexcept
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

SQLitePositionDao::SQLitePositionDao() noexcept
{}

SQLitePositionDao::~SQLitePositionDao() noexcept
{}

bool SQLitePositionDao::add(qint64 aircraftId, const PositionData &position)  noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into position ("
        "  aircraft_id,"
        "  timestamp,"
        "  latitude,"
        "  longitude,"
        "  altitude,"
        "  pitch,"
        "  bank,"
        "  heading,"
        "  velocity_x,"
        "  velocity_y,"
        "  velocity_z,"
        "  rotation_velocity_x,"
        "  rotation_velocity_y,"
        "  rotation_velocity_z"
        ") values ("
        " :aircraft_id,"
        " :timestamp,"
        " :latitude,"
        " :longitude,"
        " :altitude,"
        " :pitch,"
        " :bank,"
        " :heading,"
        " :velocity_x,"
        " :velocity_y,"
        " :velocity_z,"
        " :rotation_velocity_x,"
        " :rotation_velocity_y,"
        " :rotation_velocity_z"
        ");"
    );
    query.bindValue(":aircraft_id", aircraftId);
    query.bindValue(":timestamp", position.timestamp);
    query.bindValue(":latitude", position.latitude);
    query.bindValue(":longitude", position.longitude);
    query.bindValue(":altitude", position.altitude);
    query.bindValue(":pitch", position.pitch);
    query.bindValue(":bank", position.bank);
    query.bindValue(":heading", position.heading);
    query.bindValue(":velocity_x", position.velocityBodyX);
    query.bindValue(":velocity_y", position.velocityBodyY);
    query.bindValue(":velocity_z", position.velocityBodyZ);
    query.bindValue(":rotation_velocity_x", position.rotationVelocityBodyX);
    query.bindValue(":rotation_velocity_y", position.rotationVelocityBodyY);
    query.bindValue(":rotation_velocity_z", position.rotationVelocityBodyZ);

    bool ok = query.exec();

#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePositionDao::add: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLitePositionDao::getByAircraftId(qint64 aircraftId, std::insert_iterator<std::vector<PositionData>> insertIterator) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   position p "
        "where  p.aircraft_id = :aircraft_id "
        "order by p.timestamp asc;"
    );

    query.bindValue(":aircraft_id", aircraftId);
    bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int timestampIdx = record.indexOf("timestamp");
        const int latitudeIdx = record.indexOf("latitude");
        const int longitudeIdx = record.indexOf("longitude");
        const int altitudeIdx = record.indexOf("altitude");
        const int pitchIdx = record.indexOf("pitch");
        const int bankIdx = record.indexOf("bank");
        const int headingIdx = record.indexOf("heading");
        const int velocitXIdx = record.indexOf("velocity_x");
        const int velocitYIdx = record.indexOf("velocity_y");
        const int velocitZIdx = record.indexOf("velocity_z");
        const int rotationVelocityXIdx = record.indexOf("rotation_velocity_x");
        const int rotationVelocityYIdx = record.indexOf("rotation_velocity_y");
        const int rotationVelocityZIdx = record.indexOf("rotation_velocity_z");
        while (query.next()) {

            PositionData data;

            data.timestamp = query.value(timestampIdx).toLongLong();
            data.latitude = query.value(latitudeIdx).toDouble();
            data.longitude = query.value(longitudeIdx).toDouble();
            data.altitude = query.value(altitudeIdx).toDouble();
            data.pitch = query.value(pitchIdx).toDouble();
            data.bank = query.value(bankIdx).toDouble();
            data.heading = query.value(headingIdx).toDouble();
            data.velocityBodyX = query.value(velocitXIdx).toDouble();
            data.velocityBodyY = query.value(velocitYIdx).toDouble();
            data.velocityBodyZ = query.value(velocitZIdx).toDouble();
            data.rotationVelocityBodyX = query.value(rotationVelocityXIdx).toDouble();
            data.rotationVelocityBodyY = query.value(rotationVelocityYIdx).toDouble();
            data.rotationVelocityBodyZ = query.value(rotationVelocityZIdx).toDouble();

            insertIterator = std::move(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLitePositionDao::getByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }

    return ok;
}

bool SQLitePositionDao::deleteByFlightId(qint64 flightId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   position "
        "where  aircraft_id in (select a.id "
        "                       from aircraft a"
        "                       where a.flight_id = :flight_id"
        "                      );"
    );

    query.bindValue(":flight_id", flightId);
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePositionDao::deleteByFlightId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLitePositionDao::deleteByAircraftId(qint64 aircraftId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   position "
        "where  aircraft_id = :aircraft_id;"
    );

    query.bindValue(":aircraft_id", aircraftId);
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePositionDao::deleteByAircraftId: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
    }
#endif
    return true;
}
