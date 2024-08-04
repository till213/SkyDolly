/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#include <utility>
#include <vector>
#include <utility>

#include <QString>
#include <QStringBuilder>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDriver>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/Location.h>
#include <LocationSelector.h>
#include "SQLiteLocationDao.h"

namespace
{
    // The initial capacity of the location vector (e.g. SQLite does not support returning
    // the result count for the given SELECT query)
    constexpr int DefaultCapacity = 200;
}

struct SQLiteLocationDaoPrivate
{
    SQLiteLocationDaoPrivate(QString connectionName) noexcept
        : connectionName {std::move(connectionName)}
    {}

    QString connectionName;
};

// PUBLIC

SQLiteLocationDao::SQLiteLocationDao(QString connectionName) noexcept
    : d {std::make_unique<SQLiteLocationDaoPrivate>(std::move(connectionName))}
{}

SQLiteLocationDao::SQLiteLocationDao(SQLiteLocationDao &&rhs) noexcept = default;
SQLiteLocationDao &SQLiteLocationDao::operator=(SQLiteLocationDao &&rhs) noexcept = default;
SQLiteLocationDao::~SQLiteLocationDao() = default;

bool SQLiteLocationDao::add(Location &location) const noexcept
{
    bool ok {false};
    const auto locationId = insert(location);
    if (locationId != Const::InvalidId) {
        location.id = locationId;
        ok = true;
    }
    return ok;
}

bool SQLiteLocationDao::exportLocation(const Location &location) const noexcept
{
    const auto locationId = insert(location);
    return locationId != Const::InvalidId;
}

bool SQLiteLocationDao::update(const Location &location) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update location "
        "set    title = :title,"
        "       description = :description,"
        "       local_sim_date = :local_sim_date,"
        "       local_sim_time = :local_sim_time,"
        "       type_id = :type_id,"
        "       category_id = :category_id,"
        "       country_id = :country_id,"
        "       identifier = :identifier,"
        "       latitude = :latitude,"
        "       longitude = :longitude,"
        "       altitude = :altitude,"
        "       pitch = :pitch,"
        "       bank = :bank,"
        "       true_heading = :true_heading,"
        "       indicated_airspeed = :indicated_airspeed,"
        "       on_ground = :on_ground,"
        "       engine_event = :engine_event "
        "where id = :id;"
    );

    query.bindValue(":title", location.title);
    query.bindValue(":description", location.description);
    query.bindValue(":local_sim_date", location.localSimulationDate);
    query.bindValue(":local_sim_time", location.localSimulationTime);
    query.bindValue(":type_id", QVariant::fromValue(location.typeId));
    query.bindValue(":category_id", QVariant::fromValue(location.categoryId));
    query.bindValue(":country_id", QVariant::fromValue(location.countryId));
    query.bindValue(":identifier", location.identifier);
    query.bindValue(":latitude", location.latitude);
    query.bindValue(":longitude", location.longitude);
    query.bindValue(":altitude", location.altitude);
    query.bindValue(":pitch", location.pitch);
    query.bindValue(":bank", location.bank);
    query.bindValue(":true_heading", location.trueHeading);
    query.bindValue(":indicated_airspeed", location.indicatedAirspeed);
    query.bindValue(":on_ground", location.onGround);
    query.bindValue(":engine_event", QVariant::fromValue(location.engineEventId));
    query.bindValue(":id", QVariant::fromValue(location.id));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteLocationDao::update: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif

    return ok;
}

std::vector<Location> SQLiteLocationDao::getByPosition(double latitude, double longitude, double distanceKm, bool *ok) const noexcept
{
    std::vector<Location> locations;
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);

    // https://jonisalonen.com/2014/computing-distance-between-coordinates-can-be-simple-and-fast/
    query.prepare(
        "select * "
        "from   location l "
        "where  power(latitude - :latitude, 2) + power((longitude - :longitude) * cos(radians(:latitude)), 2) <= power(:distance / 110.25, 2) "
        "order by l.id;"
    );

    query.bindValue(":latitude", latitude);
    query.bindValue(":longitude", longitude);
    query.bindValue(":distance", distanceKm);

    const bool success = query.exec();
    if (success) {
        const auto db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            locations.reserve(query.size());
        } else {
            locations.reserve(1);
        }
        QSqlRecord record = query.record();
        const auto idIdx = record.indexOf("id");
        const auto titleIdx = record.indexOf("title");
        const auto descriptionIdx = record.indexOf("description");
        const auto localSimulationDateIdx = record.indexOf("local_sim_date");
        const auto localSimulationTimeIdx = record.indexOf("local_sim_time");
        const auto typeIdx = record.indexOf("type_id");
        const auto categoryIdx = record.indexOf("category_id");
        const auto countryIdx = record.indexOf("country_id");
        const auto identifierIdx = record.indexOf("identifier");
        const auto latitudeIdx = record.indexOf("latitude");
        const auto longitudeIdx = record.indexOf("longitude");
        const auto altitudeIdx = record.indexOf("altitude");
        const auto pitchIdx = record.indexOf("pitch");
        const auto bankIdx = record.indexOf("bank");
        const auto trueHeadingIdx = record.indexOf("true_heading");
        const auto indicatedAirspeedIdx = record.indexOf("indicated_airspeed");
        const auto onGroundIdx = record.indexOf("on_ground");
        const auto engineEventIdx = record.indexOf("engine_event");

        while (query.next()) {
            Location location;
            location.id = query.value(idIdx).toLongLong();
            location.title = query.value(titleIdx).toString();
            location.description = query.value(descriptionIdx).toString();
            // Persisted date & time are already local simulation date & time
            location.localSimulationDate = query.value(localSimulationDateIdx).toDate();
            location.localSimulationTime = query.value(localSimulationTimeIdx).toTime();
            location.typeId = query.value(typeIdx).toLongLong();
            location.categoryId = query.value(categoryIdx).toLongLong();
            location.countryId = query.value(countryIdx).toLongLong();
            location.identifier = query.value(identifierIdx).toString();
            location.latitude = query.value(latitudeIdx).toDouble();
            location.longitude = query.value(longitudeIdx).toDouble();
            location.altitude = query.value(altitudeIdx).toDouble();
            location.pitch = query.value(pitchIdx).toDouble();
            location.bank = query.value(bankIdx).toDouble();
            location.trueHeading = query.value(trueHeadingIdx).toDouble();
            location.indicatedAirspeed = query.value(indicatedAirspeedIdx).toInt();
            location.onGround = query.value(onGroundIdx).toBool();
            location.engineEventId = query.value(engineEventIdx).toLongLong();

            locations.push_back(std::move(location));
        }
    }
#ifdef DEBUG
    else {
        qDebug() << "SQLiteLocationDao::getByPosition: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    if (ok != nullptr) {
        *ok = success;
    }
    return locations;
}

bool SQLiteLocationDao::deleteById(std::int64_t id) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "delete "
        "from   location "
        "where  id = :id;"
    );
    query.bindValue(":id", QVariant::fromValue(id));

    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteLocationDao::deleteById: SQL error:" << query.lastError().text()  << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

std::vector<Location> SQLiteLocationDao::getAll(bool *ok) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(
        "select * "
        "from   location l "
        "order by l.id;"
    );

    return executeGetLocationQuery(query, ok);
}

std::vector<Location> SQLiteLocationDao::getSelectedLocations(const LocationSelector &selector, bool *ok) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);

    QString searchKeyword;
    QString queryString =
        "select * "
        "from   location l "
        "where 1 = 1 ";
    if (selector.typeSelection.size() > 0) {
        queryString.append("  and l.type_id in (");
        std::size_t i {0};
        for (const auto typeId : selector.typeSelection) {
            queryString.append(QString::number(typeId));
            if (i < selector.typeSelection.size() - 1) {
                queryString.append(",");
            }
            ++i;
        }
        queryString.append(")");
    }
    if (selector.categoryId != Const::InvalidId) {
        queryString.append("  and l.category_id = :category_id ");
    }
    if (selector.countryId != Const::InvalidId) {
        queryString.append("  and l.country_id = :country_id ");
    }
    if (!selector.searchKeyword.isEmpty()) {
        queryString.append(
            "  and (   l.title like :search_keyword "
            "       or l.description like :search_keyword "
            "       or l.identifier like :search_keyword "
            "      ) "
        );
        const QString LikeOperatorPlaceholder {"%"};
        // Add like operator placeholders
        searchKeyword = LikeOperatorPlaceholder % selector.searchKeyword % LikeOperatorPlaceholder;
    }
    queryString.append("order by l.id;");
    query.prepare(queryString);
    query.bindValue(":category_id", QVariant::fromValue(selector.categoryId));
    query.bindValue(":country_id", QVariant::fromValue(selector.countryId));
    query.bindValue(":search_keyword", searchKeyword);

    return executeGetLocationQuery(query, ok);
}

// PRIVATE

inline std::vector<Location> SQLiteLocationDao::executeGetLocationQuery(QSqlQuery &query, bool *ok) const noexcept
{
    std::vector<Location> locations;
    const bool success = query.exec();
    if (success) {
        const auto db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            locations.reserve(query.size());
        } else {
            locations.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const auto idIdx = record.indexOf("id");
        const auto titleIdx = record.indexOf("title");
        const auto descriptionIdx = record.indexOf("description");
        const auto localSimulationDateIdx = record.indexOf("local_sim_date");
        const auto localSimulationTimeIdx = record.indexOf("local_sim_time");
        const auto typeIdx = record.indexOf("type_id");
        const auto categoryIdx = record.indexOf("category_id");
        const auto countryIdx = record.indexOf("country_id");
        const auto identifierIdx = record.indexOf("identifier");
        const auto latitudeIdx = record.indexOf("latitude");
        const auto longitudeIdx = record.indexOf("longitude");
        const auto altitudeIdx = record.indexOf("altitude");
        const auto pitchIdx = record.indexOf("pitch");
        const auto bankIdx = record.indexOf("bank");
        const auto trueHeadingIdx = record.indexOf("true_heading");
        const auto indicatedAirspeedIdx = record.indexOf("indicated_airspeed");
        const auto onGroundIdx = record.indexOf("on_ground");
        const auto engineEventIdx = record.indexOf("engine_event");

        while (query.next()) {
            Location location;
            location.id = query.value(idIdx).toLongLong();
            location.title = query.value(titleIdx).toString();
            location.description = query.value(descriptionIdx).toString();
            location.localSimulationDate= query.value(localSimulationDateIdx).toDate();
            location.localSimulationTime = query.value(localSimulationTimeIdx).toTime();
            location.typeId = query.value(typeIdx).toLongLong();
            location.categoryId = query.value(categoryIdx).toLongLong();
            location.countryId = query.value(countryIdx).toLongLong();
            location.identifier = query.value(identifierIdx).toString();
            location.latitude = query.value(latitudeIdx).toDouble();
            location.longitude = query.value(longitudeIdx).toDouble();
            location.altitude = query.value(altitudeIdx).toDouble();
            location.pitch = query.value(pitchIdx).toDouble();
            location.bank = query.value(bankIdx).toDouble();
            location.trueHeading = query.value(trueHeadingIdx).toDouble();
            location.indicatedAirspeed = query.value(indicatedAirspeedIdx).toInt();
            location.onGround = query.value(onGroundIdx).toBool();
            location.engineEventId = query.value(engineEventIdx).toLongLong();

            locations.push_back(std::move(location));
        }
    }
#ifdef DEBUG
    else {
        qDebug() << "SQLiteLocationDao::executeGetLocationQuery: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    if (ok != nullptr) {
        *ok = success;
    }
    return locations;
}

std::int64_t SQLiteLocationDao::insert(const Location &location) const noexcept
{
    auto locationId {Const::InvalidId};
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "insert into location ("
        "  title,"
        "  description,"
        "  local_sim_date,"
        "  local_sim_time,"
        "  type_id,"
        "  category_id,"
        "  country_id,"
        "  identifier,"
        "  latitude,"
        "  longitude,"
        "  altitude,"
        "  pitch,"
        "  bank,"
        "  true_heading,"
        "  indicated_airspeed,"
        "  on_ground,"
        "  engine_event"
        ") values ("
        "  :title,"
        "  :description,"
        "  :local_sim_date,"
        "  :local_sim_time,"
        "  :type_id,"
        "  :category_id,"
        "  :country_id,"
        "  :identifier,"
        "  :latitude,"
        "  :longitude,"
        "  :altitude,"
        "  :pitch,"
        "  :bank,"
        "  :true_heading,"
        "  :indicated_airspeed,"
        "  :on_ground,"
        "  :engine_event"
        ");"
        );

    query.bindValue(":title", location.title);
    query.bindValue(":description", location.description);
    query.bindValue(":local_sim_date", location.localSimulationDate);
    query.bindValue(":local_sim_time", location.localSimulationTime);
    query.bindValue(":type_id", QVariant::fromValue(location.typeId));
    query.bindValue(":category_id", QVariant::fromValue(location.categoryId));
    query.bindValue(":country_id", QVariant::fromValue(location.countryId));
    query.bindValue(":identifier", location.identifier);
    query.bindValue(":latitude", location.latitude);
    query.bindValue(":longitude", location.longitude);
    query.bindValue(":altitude", location.altitude);
    query.bindValue(":pitch", location.pitch);
    query.bindValue(":bank", location.bank);
    query.bindValue(":true_heading", location.trueHeading);
    query.bindValue(":indicated_airspeed", location.indicatedAirspeed);
    query.bindValue(":on_ground", location.onGround);
    query.bindValue(":engine_event", QVariant::fromValue(location.engineEventId));

    const bool ok = query.exec();
    if (ok) {
        locationId = query.lastInsertId().toLongLong();
    } else {
        locationId = Const::InvalidId;
#ifdef DEBUG
        qDebug() << "SQLiteLocationDao::insert: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    return locationId;
}
