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
#include <iterator>

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/Location.h>
#include <LocationSelector.h>
#include "SQLiteLocationDao.h"

// PUBIC

SQLiteLocationDao::SQLiteLocationDao() noexcept
{}

SQLiteLocationDao::~SQLiteLocationDao() noexcept
{}

bool SQLiteLocationDao::add(Location &location) noexcept
{
    QSqlQuery query;
    query.prepare(
        "insert into location ("
        "  title,"
        "  description,"
        "  type_id,"
        "  category_id,"
        "  identifier,"
        "  latitude,"
        "  longitude,"
        "  altitude,"
        "  pitch,"
        "  bank,"
        "  heading,"
        "  indicated_airspeed,"
        "  on_ground"
        ") values ("
        "  :title,"
        "  :description,"
        "  :type_id,"
        "  :category_id,"
        "  :identifier,"
        "  :latitude,"
        "  :longitude,"
        "  :altitude,"
        "  :pitch,"
        "  :bank,"
        "  :heading,"
        "  :indicated_airspeed,"
        "  :on_ground"
        ");"
    );

    query.bindValue(":title", location.title);
    query.bindValue(":description", location.description);
    query.bindValue(":type_id", location.typeId);
    query.bindValue(":category_id", location.categoryId);
    query.bindValue(":identifier", location.identifier);
    query.bindValue(":latitude", location.latitude);
    query.bindValue(":longitude", location.longitude);
    query.bindValue(":altitude", location.altitude);
    query.bindValue(":pitch", location.pitch);
    query.bindValue(":bank", location.bank);
    query.bindValue(":heading", location.heading);
    query.bindValue(":indicated_airspeed", location.indicatedAirspeed);
    query.bindValue(":on_ground", location.onGround);

    bool ok = query.exec();
    if (ok) {
        std::int64_t id = query.lastInsertId().toLongLong(&ok);
        location.id = id;
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteLocationDao::add: SQL error:" << qPrintable(query.lastError().databaseText()) << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    return ok;
}

bool SQLiteLocationDao::update(const Location &location) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update location "
        "set    latitude           = :latitude,"
        "       longitude          = :longitude,"
        "       altitude           = :altitude,"
        "       pitch              = :pitch,"
        "       bank               = :bank,"
        "       heading            = :heading,"
        "       indicated_airspeed = :indicated_airspeed,"
        "       on_ground          = :on_ground,"
        "       description        = :description "
        "where id = :id;"
    );

    query.bindValue(":latitude", location.latitude);
    query.bindValue(":longitude", location.longitude);
    query.bindValue(":altitude", location.altitude);
    query.bindValue(":pitch", location.pitch);
    query.bindValue(":bank", location.bank);
    query.bindValue(":heading", location.heading);
    query.bindValue(":indicated_airspeed", location.indicatedAirspeed);
    query.bindValue(":on_ground", location.onGround);
    query.bindValue(":description", location.description);
    query.bindValue(":id", location.id);
    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteLocationDao::add: SQL error:" << qPrintable(query.lastError().databaseText()) << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif

    return ok;
}

bool SQLiteLocationDao::deleteById(std::int64_t id) noexcept
{
    QSqlQuery query;
    query.prepare(
        "delete "
        "from   location "
        "where  id = :id;"
    );
    query.bindValue(":id", QVariant::fromValue(id));

    bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteLocationDao::deleteById: SQL error:" << qPrintable(query.lastError().databaseText())  << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

bool SQLiteLocationDao::getAll(std::back_insert_iterator<std::vector<Location>> backInsertIterator) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   location l "
        "order by l.id;"
    );

    bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int idIdx = record.indexOf("id");
        const int latitudeIdx = record.indexOf("latitude");
        const int longitudeIdx = record.indexOf("longitude");
        const int altitudeIdx = record.indexOf("altitude");
        const int pitchIdx = record.indexOf("pitch");
        const int bankIdx = record.indexOf("bank");
        const int headingIdx = record.indexOf("heading");
        const int indicatedAirspeedIdx = record.indexOf("indicated_airspeed");
        const int onGroundIdx = record.indexOf("on_ground");
        const int descriptionIdx = record.indexOf("description");

        while (ok && query.next()) {
            Location location;
            location.id = query.value(idIdx).toLongLong();
            location.latitude = query.value(latitudeIdx).toDouble();
            location.longitude = query.value(longitudeIdx).toDouble();
            location.altitude = query.value(altitudeIdx).toDouble();
            location.pitch = query.value(pitchIdx).toDouble();
            location.bank = query.value(bankIdx).toDouble();
            location.heading = query.value(headingIdx).toDouble();
            location.indicatedAirspeed = query.value(indicatedAirspeedIdx).toInt();
            location.onGround = query.value(onGroundIdx).toBool();
            location.description = query.value(descriptionIdx).toString();
            backInsertIterator = std::move(location);
        }
    }
#ifdef DEBUG
    else {
        qDebug("SQLiteLocationDao::getAll: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code:" + query.lastError().nativeErrorCode()));
    }
#endif
    return ok;
}

bool SQLiteLocationDao::getSelectedLocations(const LocationSelector &selector, std::back_insert_iterator<std::vector<Location>> backInsertIterator) const noexcept
{
    // TODO IMPLEMENT ME
    return true;
}
