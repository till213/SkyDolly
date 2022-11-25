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
#include <cstdint>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QTimeZone>
#include <QDateTime>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Settings.h>
#include <Kernel/Version.h>
#include <Metadata.h>
#include "SqlMigration.h"
#include "SQLiteDatabaseDao.h"

namespace
{
    constexpr const char *DbName {"QSQLITE"};
}

struct DatabaseDaoPrivate
{
    QSqlDatabase db;
};

// PUBLIC

SQLiteDatabaseDao::SQLiteDatabaseDao() noexcept
    : d(std::make_unique<DatabaseDaoPrivate>())
{}

SQLiteDatabaseDao::SQLiteDatabaseDao(SQLiteDatabaseDao &&rhs) noexcept = default;
SQLiteDatabaseDao &SQLiteDatabaseDao::operator=(SQLiteDatabaseDao &&rhs) noexcept = default;

SQLiteDatabaseDao::~SQLiteDatabaseDao()
{
    disconnectSQLite();
}

bool SQLiteDatabaseDao::connectDb(const QString &logbookPath) noexcept
{
    d->db = QSqlDatabase::addDatabase(::DbName);
    d->db.setDatabaseName(logbookPath);
    return d->db.open();
}

void SQLiteDatabaseDao::disconnectDb() noexcept
{
    disconnectSQLite();
}

bool SQLiteDatabaseDao::migrate() noexcept
{
    bool ok = createMigrationTable();
    if (ok) {
        SqlMigration sqlMigration;
        ok = sqlMigration.migrate();
    }
    return ok;
}

bool SQLiteDatabaseDao::optimise() noexcept
{
    QSqlQuery query;
    bool ok = query.exec("vacuum;");
    if (ok) {
        ok = query.exec("update metadata set last_optim_date = datetime('now') where rowid = 1;");
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteDatabaseDao::optimise(: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool SQLiteDatabaseDao::backup(const QString &backupPath) noexcept
{
    QSqlQuery query;
    bool ok = query.exec(QString("vacuum into '%1';").arg(backupPath));
    if (ok) {
        ok = query.exec("update metadata set last_backup_date = datetime('now') where rowid = 1;");
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteDatabaseDao::backup(: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool SQLiteDatabaseDao::updateBackupPeriod(std::int64_t backupPeriodId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update metadata "
        "set    backup_period_id = :backup_period_id;"
    );

    query.bindValue(":backup_period_id", QVariant::fromValue(backupPeriodId));
    return query.exec();
}

bool SQLiteDatabaseDao::updateNextBackupDate(const QDateTime &date) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update metadata "
        "set    next_backup_date = :next_backup_date;"
    );

    query.bindValue(":next_backup_date", date.toUTC());
    return query.exec();
}

bool SQLiteDatabaseDao::updateBackupDirectoryPath(const QString &backupDirectoryPath) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update metadata "
        "set    backup_directory_path = :backup_directory_path;"
    );

    query.bindValue(":backup_directory_path", backupDirectoryPath);
    return query.exec();
}

Metadata SQLiteDatabaseDao::getMetadata(bool *ok) const noexcept
{
    Metadata metadata;
    QSqlQuery query;
    query.setForwardOnly(true);

    const bool success = query.exec(
        "select m.creation_date,"
        "       m.app_version,"
        "       m.last_optim_date,"
        "       m.last_backup_date,"
        "       m.next_backup_date,"
        "       m.backup_directory_path,"
        "       m.backup_period_id "
        "from metadata m;"
    );
    if (success && query.next()) {
        QDateTime dateTime = query.value(0).toDateTime();
        dateTime.setTimeZone(QTimeZone::utc());
        metadata.creationDate = dateTime.toLocalTime();

        QString appVersion = query.value(1).toString();
        metadata.appVersion.fromString(appVersion);

        dateTime = query.value(2).toDateTime();
        dateTime.setTimeZone(QTimeZone::utc());
        metadata.lastOptimisationDate = dateTime.toLocalTime();

        dateTime = query.value(3).toDateTime();
        dateTime.setTimeZone(QTimeZone::utc());
        metadata.lastBackupDate = dateTime.toLocalTime();

        dateTime = query.value(4).toDateTime();
        dateTime.setTimeZone(QTimeZone::utc());
        metadata.nextBackupDate = dateTime.toLocalTime();

        metadata.backupDirectoryPath = query.value(5).toString();
        metadata.backupPeriodId = query.value(6).toLongLong();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return metadata;
};

Version SQLiteDatabaseDao::getDatabaseVersion(bool *ok) const noexcept
{
    Version version;
    QSqlQuery query;
    const bool success = query.exec("select m.app_version from metadata m;");
    if (success && query.next()) {
        QString appVersion = query.value(0).toString();
        version.fromString(appVersion);
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return version;
};

QString SQLiteDatabaseDao::getBackupDirectoryPath(bool *ok) const noexcept
{
    QString backupDirectoryPath;
    QSqlQuery query;
    const bool success = query.exec("select m.backup_directory_path from metadata m;");
    if (ok && query.next()) {
        backupDirectoryPath = query.value(0).toString();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return backupDirectoryPath;
};

// PRIVATE

void SQLiteDatabaseDao::disconnectSQLite() noexcept
{
    d->db.close();
}

bool SQLiteDatabaseDao::createMigrationTable() noexcept
{
    QSqlQuery query;
    query.prepare(
        "create table if not exists migr("
        "id text not null,"
        "step integer not null,"
        "success integer not null,"
        "timestamp datetime default current_timestamp,"
        "msg text,"
        "primary key (id, step));"
    );
    return query.exec();
}
