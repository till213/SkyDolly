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
#include <QStringLiteral>
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
    constexpr int DefaultCapacity = 25;
}

struct SQLiteLocationDaoPrivate
{
    SQLiteLocationDaoPrivate(QString connectionName) noexcept
        : connectionName(std::move(connectionName))
    {}

    QString connectionName;
};

// PUBLIC

SQLiteLocationDao::SQLiteLocationDao(QString connectionName) noexcept
    : d(std::make_unique<SQLiteLocationDaoPrivate>(std::move(connectionName)))
{}

SQLiteLocationDao::SQLiteLocationDao(SQLiteLocationDao &&rhs) noexcept = default;
SQLiteLocationDao &SQLiteLocationDao::operator=(SQLiteLocationDao &&rhs) noexcept = default;
SQLiteLocationDao::~SQLiteLocationDao() = default;

bool SQLiteLocationDao::add(Location &location) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(QStringLiteral(
        "insert into location ("
        "  title,"
        "  description,"
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
        "  attributes,"
        "  engine_event"
        ") values ("
        "  :title,"
        "  :description,"
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
        "  :attributes,"
        "  :engine_event"
        ");"
    ));

    query.bindValue(QStringLiteral(":title"), location.title);
    query.bindValue(QStringLiteral(":description"), location.description);
    query.bindValue(QStringLiteral(":type_id"), QVariant::fromValue(location.typeId));
    query.bindValue(QStringLiteral(":category_id"), QVariant::fromValue(location.categoryId));
    query.bindValue(QStringLiteral(":country_id"), QVariant::fromValue(location.countryId));
    query.bindValue(QStringLiteral(":identifier"), location.identifier);
    query.bindValue(QStringLiteral(":latitude"), location.latitude);
    query.bindValue(QStringLiteral(":longitude"), location.longitude);
    query.bindValue(QStringLiteral(":altitude"), location.altitude);
    query.bindValue(QStringLiteral(":pitch"), location.pitch);
    query.bindValue(QStringLiteral(":bank"), location.bank);
    query.bindValue(QStringLiteral(":true_heading"), location.trueHeading);
    query.bindValue(QStringLiteral(":indicated_airspeed"), location.indicatedAirspeed);
    query.bindValue(QStringLiteral(":on_ground"), location.onGround);
    query.bindValue(QStringLiteral(":attributes"), QVariant::fromValue(location.attributes));
    query.bindValue(QStringLiteral(":engine_event"), QVariant::fromValue(location.engineEventId));

    const bool ok = query.exec();
    if (ok) {
        location.id = query.lastInsertId().toLongLong();
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteLocationDao::add: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    return ok;
}

bool SQLiteLocationDao::update(const Location &location) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(QStringLiteral(
        "update location "
        "set    title = :title,"
        "       description = :description,"
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
        "       attributes = :attributes,"
        "       engine_event = :engine_event "
        "where id = :id;"
    ));

    query.bindValue(QStringLiteral(":title"), location.title);
    query.bindValue(QStringLiteral(":description"), location.description);
    query.bindValue(QStringLiteral(":type_id"), QVariant::fromValue(location.typeId));
    query.bindValue(QStringLiteral(":category_id"), QVariant::fromValue(location.categoryId));
    query.bindValue(QStringLiteral(":country_id"), QVariant::fromValue(location.countryId));
    query.bindValue(QStringLiteral(":identifier"), location.identifier);
    query.bindValue(QStringLiteral(":latitude"), location.latitude);
    query.bindValue(QStringLiteral(":longitude"), location.longitude);
    query.bindValue(QStringLiteral(":altitude"), location.altitude);
    query.bindValue(QStringLiteral(":pitch"), location.pitch);
    query.bindValue(QStringLiteral(":bank"), location.bank);
    query.bindValue(QStringLiteral(":true_heading"), location.trueHeading);
    query.bindValue(QStringLiteral(":indicated_airspeed"), location.indicatedAirspeed);
    query.bindValue(QStringLiteral(":on_ground"), location.onGround);
    query.bindValue(QStringLiteral(":attributes"), QVariant::fromValue(location.attributes));
    query.bindValue(QStringLiteral(":engine_event"), QVariant::fromValue(location.engineEventId));
    query.bindValue(QStringLiteral(":id"), QVariant::fromValue(location.id));
    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteLocationDao::update: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif

    return ok;
}

std::vector<Location> SQLiteLocationDao::getByPosition(double latitude, double longitude, double distance, bool *ok) const noexcept
{
    std::vector<Location> locations;
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);

    // TODO Implement https://jonisalonen.com/2014/computing-distance-between-coordinates-can-be-simple-and-fast/
    //      SQlite supports "math functions" from Qt 6.5 onwards:
    //      - https://bugreports.qt.io/browse/QTBUG-108016
    //      - https://codereview.qt-project.org/c/qt/qtbase/+/440378
    query.prepare(QStringLiteral(
        "select * "
        "from   location l "
        "where  l.latitude = :latitude "
        "  and  l.longitude = :longitude "
        "order by l.id;"
    ));

    query.bindValue(QStringLiteral(":latitude"), latitude);
    query.bindValue(QStringLiteral(":longitude"), longitude);

    const bool success = query.exec();
    if (success) {
        const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            locations.reserve(query.size());
        } else {
            locations.reserve(1);
        }
        QSqlRecord record = query.record();
        const int idIdx = record.indexOf(QStringLiteral("id"));
        const int titleIdx = record.indexOf(QStringLiteral("title"));
        const int descriptionIdx = record.indexOf(QStringLiteral("description"));
        const int typeIdx = record.indexOf(QStringLiteral("type_id"));
        const int categoryIdx = record.indexOf(QStringLiteral("category_id"));
        const int countryIdx = record.indexOf(QStringLiteral("country_id"));
        const int identifierIdx = record.indexOf(QStringLiteral("identifier"));
        const int latitudeIdx = record.indexOf(QStringLiteral("latitude"));
        const int longitudeIdx = record.indexOf(QStringLiteral("longitude"));
        const int altitudeIdx = record.indexOf(QStringLiteral("altitude"));
        const int pitchIdx = record.indexOf(QStringLiteral("pitch"));
        const int bankIdx = record.indexOf(QStringLiteral("bank"));
        const int trueHeadingIdx = record.indexOf(QStringLiteral("true_heading"));
        const int indicatedAirspeedIdx = record.indexOf(QStringLiteral("indicated_airspeed"));
        const int onGroundIdx = record.indexOf(QStringLiteral("on_ground"));
        const int attributesIdx = record.indexOf(QStringLiteral("attributes"));
        const int engineEventIdx = record.indexOf(QStringLiteral("engine_event"));

        while (query.next()) {
            Location location;
            location.id = query.value(idIdx).toLongLong();
            location.title = query.value(titleIdx).toString();
            location.description = query.value(descriptionIdx).toString();
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
            location.attributes = query.value(attributesIdx).toLongLong();
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
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(QStringLiteral(
        "delete "
        "from   location "
        "where  id = :id;"
    ));
    query.bindValue(QStringLiteral(":id"), QVariant::fromValue(id));

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
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(
        "select * "
        "from   location l "
        "order by l.id;"
    ));

    return executeGetLocationQuery(query, ok);
}

std::vector<Location> SQLiteLocationDao::getSelectedLocations(const LocationSelector &selector, bool *ok) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);

    QString searchKeyword;
    QString queryString = QStringLiteral(
        "select * "
        "from   location l "
        "where 1 = 1 "
    );
    if (selector.typeSelection.size() > 0) {
        queryString.append(QStringLiteral("  and l.type_id in ("));
        std::size_t i {0};
        for (const auto typeId : selector.typeSelection) {
            queryString.append(QString::number(typeId));
            if (i < selector.typeSelection.size() - 1) {
                queryString.append(QStringLiteral(","));
            }
            ++i;
        }
        queryString.append(QStringLiteral(")"));
    }
    if (selector.categoryId != Const::InvalidId) {
        queryString.append(QStringLiteral("  and l.category_id = :category_id "));
    }
    if (selector.countryId != Const::InvalidId) {
        queryString.append(QStringLiteral("  and l.country_id = :country_id "));
    }
    if (!selector.searchKeyword.isEmpty()) {
        queryString.append(QStringLiteral(
            "  and (   l.title like :search_keyword "
            "       or l.description like :search_keyword "
            "       or l.identifier like :search_keyword "
            "      ) "
        )

        );
        const QString LikeOperatorPlaceholder {QStringLiteral("%")};
        // Add like operator placeholders
        searchKeyword = LikeOperatorPlaceholder  % selector.searchKeyword % LikeOperatorPlaceholder;
    }
    queryString.append(QStringLiteral("order by l.id;"));
    query.prepare(queryString);
    query.bindValue(QStringLiteral(":category_id"), QVariant::fromValue(selector.categoryId));
    query.bindValue(QStringLiteral(":country_id"), QVariant::fromValue(selector.countryId));
    query.bindValue(QStringLiteral(":search_keyword"), searchKeyword);

    return executeGetLocationQuery(query, ok);
}

// PRIVATE

inline std::vector<Location> SQLiteLocationDao::executeGetLocationQuery(QSqlQuery &query, bool *ok) const noexcept
{
    std::vector<Location> locations;
    const bool success = query.exec();
    if (success) {
        const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            locations.reserve(query.size());
        } else {
            locations.reserve(::DefaultCapacity);
        }
        QSqlRecord record = query.record();
        const int idIdx = record.indexOf(QStringLiteral("id"));
        const int titleIdx = record.indexOf(QStringLiteral("title"));
        const int descriptionIdx = record.indexOf(QStringLiteral("description"));
        const int typeIdx = record.indexOf(QStringLiteral("type_id"));
        const int categoryIdx = record.indexOf(QStringLiteral("category_id"));
        const int countryIdx = record.indexOf(QStringLiteral("country_id"));
        const int identifierIdx = record.indexOf(QStringLiteral("identifier"));
        const int latitudeIdx = record.indexOf(QStringLiteral("latitude"));
        const int longitudeIdx = record.indexOf(QStringLiteral("longitude"));
        const int altitudeIdx = record.indexOf(QStringLiteral("altitude"));
        const int pitchIdx = record.indexOf(QStringLiteral("pitch"));
        const int bankIdx = record.indexOf(QStringLiteral("bank"));
        const int trueHeadingIdx = record.indexOf(QStringLiteral("true_heading"));
        const int indicatedAirspeedIdx = record.indexOf(QStringLiteral("indicated_airspeed"));
        const int onGroundIdx = record.indexOf(QStringLiteral("on_ground"));
        const int attributesIdx = record.indexOf(QStringLiteral("attributes"));
        const int engineEventIdx = record.indexOf(QStringLiteral("engine_event"));

        while (query.next()) {
            Location location;
            location.id = query.value(idIdx).toLongLong();
            location.title = query.value(titleIdx).toString();
            location.description = query.value(descriptionIdx).toString();
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
            location.attributes = query.value(attributesIdx).toLongLong();
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
