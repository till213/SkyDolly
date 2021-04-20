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
#include "../../AircraftData.h"
#include "SQLitePositionDao.h"

class SQLitePositionDaoPrivate
{
public:
    SQLitePositionDaoPrivate() noexcept
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
        if (selectQuery == nullptr) {
            selectQuery = std::make_unique<QSqlQuery>();
            selectQuery->prepare("select a.name from aircraft a where a.id = :id;");
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

bool SQLitePositionDao::addPosition(qint64 aircraftId, const AircraftData &position)  noexcept
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
        qDebug("addPosition: SQL error: %s", qPrintable(d->insertQuery->lastError().databaseText() + " - error code: " + d->insertQuery->lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

AircraftData SQLitePositionDao::getPosition(qint64 id) const noexcept
{
    return AircraftData();
}
