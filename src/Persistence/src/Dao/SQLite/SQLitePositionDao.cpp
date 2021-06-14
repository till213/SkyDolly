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
            insertQuery->prepare(
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
");");
        }
        if (selectByAircraftIdQuery == nullptr) {
            selectByAircraftIdQuery = std::make_unique<QSqlQuery>();
            selectByAircraftIdQuery->prepare(
"select * "
"from   position p "
"where  p.aircraft_id = :aircraft_id "
"order by p.timestamp asc;");
        }
        if (deleteByFlightIdQuery == nullptr) {
            deleteByFlightIdQuery = std::make_unique<QSqlQuery>();
            deleteByFlightIdQuery->prepare(
"delete "
"from   position "
"where  aircraft_id in (select a.id "
"                       from aircraft a"
"                       where a.flight_id = :flight_id"
"                      );");
        }
        if (deleteByIdQuery == nullptr) {
            deleteByIdQuery = std::make_unique<QSqlQuery>();
            deleteByIdQuery->prepare(
"delete "
"from   position "
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

SQLitePositionDao::SQLitePositionDao(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SQLitePositionDaoPrivate>())
{
    frenchConnection();
}

SQLitePositionDao::~SQLitePositionDao() noexcept
{}

bool SQLitePositionDao::add(qint64 aircraftId, const PositionData &position)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId);
    d->insertQuery->bindValue(":timestamp", position.timestamp);
    d->insertQuery->bindValue(":latitude", position.latitude);
    d->insertQuery->bindValue(":longitude", position.longitude);
    d->insertQuery->bindValue(":altitude", position.altitude);
    d->insertQuery->bindValue(":pitch", position.pitch);
    d->insertQuery->bindValue(":bank", position.bank);
    d->insertQuery->bindValue(":heading", position.heading);
    d->insertQuery->bindValue(":velocity_x", position.velocityBodyX);
    d->insertQuery->bindValue(":velocity_y", position.velocityBodyY);
    d->insertQuery->bindValue(":velocity_z", position.velocityBodyZ);
    d->insertQuery->bindValue(":rotation_velocity_x", position.rotationVelocityBodyX);
    d->insertQuery->bindValue(":rotation_velocity_y", position.rotationVelocityBodyY);
    d->insertQuery->bindValue(":rotation_velocity_z", position.rotationVelocityBodyZ);

    bool ok = d->insertQuery->exec();

#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePositionDao::add: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLitePositionDao::getByAircraftId(qint64 aircraftId, QVector<PositionData> &positionData) const noexcept
{
    d->initQueries();
    d->selectByAircraftIdQuery->bindValue(":aircraft_id", aircraftId);
    bool ok = d->selectByAircraftIdQuery->exec();
    if (ok) {
        positionData.clear();
        QSqlRecord record = d->selectByAircraftIdQuery->record();
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
        while (d->selectByAircraftIdQuery->next()) {

            PositionData data;

            data.timestamp = d->selectByAircraftIdQuery->value(timestampIdx).toLongLong();
            data.latitude = d->selectByAircraftIdQuery->value(latitudeIdx).toDouble();
            data.longitude = d->selectByAircraftIdQuery->value(longitudeIdx).toDouble();
            data.altitude = d->selectByAircraftIdQuery->value(altitudeIdx).toDouble();
            data.pitch = d->selectByAircraftIdQuery->value(pitchIdx).toDouble();
            data.bank = d->selectByAircraftIdQuery->value(bankIdx).toDouble();
            data.heading = d->selectByAircraftIdQuery->value(headingIdx).toDouble();
            data.velocityBodyX = d->selectByAircraftIdQuery->value(velocitXIdx).toDouble();
            data.velocityBodyY = d->selectByAircraftIdQuery->value(velocitYIdx).toDouble();
            data.velocityBodyZ = d->selectByAircraftIdQuery->value(velocitZIdx).toDouble();
            data.rotationVelocityBodyX = d->selectByAircraftIdQuery->value(rotationVelocityXIdx).toDouble();
            data.rotationVelocityBodyY = d->selectByAircraftIdQuery->value(rotationVelocityYIdx).toDouble();
            data.rotationVelocityBodyZ = d->selectByAircraftIdQuery->value(rotationVelocityZIdx).toDouble();

            positionData.append(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLitePositionDao::getByAircraftId: SQL error: %s", qPrintable(d->selectByAircraftIdQuery->lastError().databaseText() + " - error code: " + d->selectByAircraftIdQuery->lastError().nativeErrorCode()));
#endif
    }

    return ok;
}

bool SQLitePositionDao::deleteByFlightId(qint64 flightId) noexcept
{
    d->initQueries();
    d->deleteByFlightIdQuery->bindValue(":flight_id", flightId);
    bool ok = d->deleteByFlightIdQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePositionDao::deleteByFlightId: SQL error: %s", qPrintable(d->deleteByFlightIdQuery->lastError().databaseText() + " - error code: " + d->deleteByFlightIdQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLitePositionDao::deleteByAircraftId(qint64 aircraftId) noexcept
{
    d->initQueries();
    d->deleteByIdQuery->bindValue(":aircraft_id", aircraftId);
    bool ok = d->deleteByIdQuery->exec();
#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePositionDao::deleteByAircraftId: SQL error: %s", qPrintable(d->deleteByIdQuery->lastError().databaseText() + " - error code: " + d->deleteByIdQuery->lastError().nativeErrorCode()));
    }
#endif
    return true;
}

// PRIVATE

void SQLitePositionDao::frenchConnection() noexcept
{
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &SQLitePositionDao::handleConnectionChanged);
}

// PRIVATE SLOTS

void SQLitePositionDao::handleConnectionChanged() noexcept
{
    d->resetQueries();
}

