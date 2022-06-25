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
#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/Location.h>
#include "SQLiteLocationDao.h"

// PUBIC

SQLiteLocationDao::SQLiteLocationDao()
{

}

bool SQLiteLocationDao::add(Location &location) noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into location ("
        "  latitude,"
        "  longitude,"
        "  altitude,"
        "  pitch,"
        "  bank,"
        "  heading,"
        "  on_ground,"
        "  description"
        ") values ("
        "  :latitude,"
        "  :longitude,"
        "  :altitude,"
        "  :pitch,"
        "  :bank,"
        "  :heading,"
        "  :on_ground,"
        "  :description"
        ");"
    );

    query.bindValue(":latitude", location.latitude);
    query.bindValue(":longitude", location.longitude);
    query.bindValue(":altitude", location.altitude);
    query.bindValue(":pitch", location.pitch);
    query.bindValue(":bank", location.bank);
    query.bindValue(":heading", location.heading);
    query.bindValue(":on_ground", location.onGround);
    query.bindValue(":description", location.description);
    bool ok = query.exec();
    if (ok) {
        std::int64_t id = query.lastInsertId().toLongLong(&ok);
        location.id = id;
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteLocationDao::add: SQL error: " << qPrintable(query.lastError().databaseText())  << " - error code: " << query.lastError().nativeErrorCode();
#endif
    }

    return ok;
}

bool SQLiteLocationDao::get(std::int64_t id, Location &location) const noexcept
{
    // TODO IMPLEMENT ME
    return true;
}

bool SQLiteLocationDao::deleteById(std::int64_t id) noexcept
{
    // TODO IMPLEMENT ME
    return true;
}
