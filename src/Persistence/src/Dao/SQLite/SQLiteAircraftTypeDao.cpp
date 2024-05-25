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

#include <Kernel/Enum.h>
#include <Model/AircraftType.h>
#include "SQLiteAircraftTypeDao.h"

namespace
{
    // The initial capacity of the aircraft type vector; happens to be the number of default
    // aircraft types (as per Sky Dolly v0.13), also refer to LogbookMigration.sql
    constexpr int DefaultCapacity = 279;
}

struct SQLiteAircraftTypeDaoPrivate
{
    SQLiteAircraftTypeDaoPrivate(QString connectionName) noexcept
        : connectionName {std::move(connectionName)}
    {}

    QString connectionName;
};

// PUBLIC

SQLiteAircraftTypeDao::SQLiteAircraftTypeDao(QString connectionName) noexcept
    : d {std::make_unique<SQLiteAircraftTypeDaoPrivate>(std::move(connectionName))}
{}

SQLiteAircraftTypeDao::SQLiteAircraftTypeDao(SQLiteAircraftTypeDao &&rhs) noexcept = default;
SQLiteAircraftTypeDao &SQLiteAircraftTypeDao::operator=(SQLiteAircraftTypeDao &&rhs) noexcept = default;
SQLiteAircraftTypeDao::~SQLiteAircraftTypeDao() = default;

bool SQLiteAircraftTypeDao::upsert(const AircraftType &aircraftType) const noexcept
{
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(QStringLiteral(
        "insert into aircraft_type (type, category, wing_span, engine_type, nof_engines) "
        "values(:type, :category, :wing_span, :engine_type, :nof_engines) "
        "on conflict(type) "
        "do update "
        "set category = excluded.category, "
        "    wing_span = excluded.wing_span, "
        "    engine_type = excluded.engine_type, "
        "    nof_engines = excluded.nof_engines;"
    ));

    query.bindValue(QStringLiteral(":type"), aircraftType.type);
    query.bindValue(QStringLiteral(":category"), aircraftType.category);
    query.bindValue(QStringLiteral(":wing_span"), aircraftType.wingSpan);
    query.bindValue(QStringLiteral(":engine_type"), Enum::underly(aircraftType.engineType));
    query.bindValue(QStringLiteral(":nof_engines"), aircraftType.numberOfEngines);

    const bool ok = query.exec();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "SQLiteAircraftTypeDao::upsert: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return ok;
}

AircraftType SQLiteAircraftTypeDao::getByType(const QString &type, bool *ok) const noexcept
{
    AircraftType aircraftType;
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(
        "select at.category, at.wing_span, at.engine_type, at.nof_engines "
         "from   aircraft_type at "
         "where  at.type = :type;"
    ));

    query.bindValue(QStringLiteral(":type"), type);
    aircraftType.type = type;
    bool success = query.exec();
    if (success) {
        const QSqlRecord record = query.record();
        success = query.next();
        if (success) {
            const int categoryIdx = record.indexOf(QStringLiteral("category"));
            const int wingSpanIdx = record.indexOf(QStringLiteral("wing_span"));
            const int engineTypeIdx = record.indexOf(QStringLiteral("engine_type"));
            const int nofEnginesIdx = record.indexOf(QStringLiteral("nof_engines"));
            aircraftType.category = query.value(categoryIdx).toString();
            aircraftType.wingSpan = query.value(wingSpanIdx).toInt();
            const auto enumValue = query.value(engineTypeIdx).toInt();
            aircraftType.engineType = Enum::contains<SimType::EngineType>(enumValue) ? static_cast<SimType::EngineType>(enumValue) : SimType::EngineType::First;
            aircraftType.numberOfEngines = query.value(nofEnginesIdx).toInt();
        }
    }
#ifdef DEBUG
    else {
        qDebug() << "SQLiteAircraftTypeDao::getByType: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftType;
}

std::vector<AircraftType> SQLiteAircraftTypeDao::getAll(bool *ok) const noexcept
{
    std::vector<AircraftType> aircraftTypes;
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(
        "select * "
        "from   aircraft_type at "
        "order by at.type asc;"
    ));
    const bool success = query.exec();
    if (success) {
        const bool querySizeFeature = db.driver()->hasFeature(QSqlDriver::QuerySize);
        if (querySizeFeature) {
            aircraftTypes.reserve(query.size());
        } else {
            aircraftTypes.reserve(::DefaultCapacity);
        }

        QSqlRecord record = query.record();
        const int typeIdx = record.indexOf(QStringLiteral("type"));
        const int categoryIdx = record.indexOf(QStringLiteral("category"));
        const int wingSpanIdx = record.indexOf(QStringLiteral("wing_span"));
        const int engineTypeIdx = record.indexOf(QStringLiteral("engine_type"));
        const int nofEnginesIdx = record.indexOf(QStringLiteral("nof_engines"));
        while (query.next()) {
            const QString type = query.value(typeIdx).toString();
            const QString category = query.value(categoryIdx).toString();
            const int wingSpan = query.value(wingSpanIdx).toInt();            
            const auto enumValue = query.value(engineTypeIdx).toInt();
            const SimType::EngineType engineType = Enum::contains<SimType::EngineType>(enumValue) ? static_cast<SimType::EngineType>(enumValue) : SimType::EngineType::First;
            const int numberOfEngines = query.value(nofEnginesIdx).toInt();
            aircraftTypes.emplace_back(type, category, wingSpan, engineType, numberOfEngines);
        }
#ifdef DEBUG
    } else {
            qDebug() << "SQLiteAircraftTypeDao::getAll: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return aircraftTypes;
}

bool SQLiteAircraftTypeDao::exists(const QString &type) const noexcept
{
    bool exists {false};
    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(
        "select count(*) "
        "from   aircraft_type at "
        "where  at.type = :type "
        "limit 1;"
    ));

    query.bindValue(QStringLiteral(":type"), type);
    const bool ok = query.exec();
    if (ok && query.next()) {
        const int count = query.value(0).toInt();
        exists = count > 0;
    }
#ifdef DEBUG
    else {
        qDebug() << "SQLiteAircraftTypeDao::exists: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif

    return ok && exists;
}
