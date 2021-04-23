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

#include "../../../../Kernel/src/Enum.h"
#include "../../AircraftData.h"
#include "SQLitePositionDao.h"

class SQLitePositionDaoPrivate
{
public:
    SQLitePositionDaoPrivate() noexcept
        : insertQuery(nullptr),
          selectByAircraftIdQuery(nullptr)
    {
    }

    std::unique_ptr<QSqlQuery> insertQuery;
    std::unique_ptr<QSqlQuery> selectByAircraftIdQuery;

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
    }
};

// PUBLIC

SQLitePositionDao::SQLitePositionDao() noexcept
    : d(std::make_unique<SQLitePositionDaoPrivate>())
{
}

SQLitePositionDao::~SQLitePositionDao() noexcept
{}

bool SQLitePositionDao::add(qint64 aircraftId, const AircraftData &position)  noexcept
{
    d->initQueries();
    d->insertQuery->bindValue(":aircraft_id", aircraftId, QSql::In);
    d->insertQuery->bindValue(":timestamp", position.timestamp, QSql::In);
    d->insertQuery->bindValue(":latitude", position.latitude, QSql::In);
    d->insertQuery->bindValue(":longitude", position.longitude, QSql::In);
    d->insertQuery->bindValue(":altitude", position.altitude, QSql::In);
    d->insertQuery->bindValue(":pitch", position.pitch, QSql::In);
    d->insertQuery->bindValue(":bank", position.bank, QSql::In);
    d->insertQuery->bindValue(":heading", position.heading, QSql::In);
    d->insertQuery->bindValue(":velocity_x", position.velocityBodyX, QSql::In);
    d->insertQuery->bindValue(":velocity_y", position.velocityBodyY, QSql::In);
    d->insertQuery->bindValue(":velocity_z", position.velocityBodyZ, QSql::In);
    d->insertQuery->bindValue(":rotation_velocity_x", position.rotationVelocityBodyX, QSql::In);
    d->insertQuery->bindValue(":rotation_velocity_y", position.rotationVelocityBodyY, QSql::In);
    d->insertQuery->bindValue(":rotation_velocity_z", position.rotationVelocityBodyZ, QSql::In);

    bool ok = d->insertQuery->exec();

#ifdef DEBUG
    if (!ok) {
        qDebug("SQLitePositionDao::add: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLitePositionDao::getByAircraftId(qint64 aircraftId, QVector<AircraftData> &aircraftData) const noexcept
{
    d->initQueries();
    d->selectByAircraftIdQuery->bindValue(":aircraft_id", aircraftId);
    bool ok = d->selectByAircraftIdQuery->exec();
    if (ok) {
        aircraftData.clear();
        int timestampIdx = d->selectByAircraftIdQuery->record().indexOf("timestamp");
        int latitudeIdx = d->selectByAircraftIdQuery->record().indexOf("latitude");
        int longitudeIdx = d->selectByAircraftIdQuery->record().indexOf("longitude");
        int altitudeIdx = d->selectByAircraftIdQuery->record().indexOf("altitude");
        int pitchIdx = d->selectByAircraftIdQuery->record().indexOf("pitch");
        int bankIdx = d->selectByAircraftIdQuery->record().indexOf("bank");
        int headingIdx = d->selectByAircraftIdQuery->record().indexOf("heading");
        int velocitXIdx = d->selectByAircraftIdQuery->record().indexOf("velocity_x");
        int velocitYIdx = d->selectByAircraftIdQuery->record().indexOf("velocity_y");
        int velocitZIdx = d->selectByAircraftIdQuery->record().indexOf("velocity_z");
        int rotationVelocityXIdx = d->selectByAircraftIdQuery->record().indexOf("rotation_velocity_x");
        int rotationVelocityYIdx = d->selectByAircraftIdQuery->record().indexOf("rotation_velocity_y");
        int rotationVelocityZIdx = d->selectByAircraftIdQuery->record().indexOf("rotation_velocity_z");
        while (d->selectByAircraftIdQuery->next()) {

            AircraftData data;

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

            aircraftData.append(data);
        }
#ifdef DEBUG
    } else {
        qDebug("SQLitePositionDao::getByAircraftId: SQL error: %s", qPrintable(d->selectByAircraftIdQuery->lastError().databaseText() + " - error code: " + d->selectByAircraftIdQuery->lastError().nativeErrorCode()));
#endif
    }

    return ok;
}
