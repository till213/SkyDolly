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

constexpr char DbName[] = "QSQLITE";

struct DatabaseDaoPrivate
{
    DatabaseDaoPrivate()
    {}

    QSqlDatabase db;
};

// PUBLIC

SQLiteDatabaseDao::SQLiteDatabaseDao() noexcept
    : d(std::make_unique<DatabaseDaoPrivate>())
{}

SQLiteDatabaseDao::~SQLiteDatabaseDao() noexcept
{
    disconnectSQLite();
}

bool SQLiteDatabaseDao::connectDb(const QString &logbookPath) noexcept
{
    d->db = QSqlDatabase::addDatabase(DbName);
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
        qDebug() << "SQLiteDatabaseDao::optimise(: SQL error:" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
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
        qDebug() << "SQLiteDatabaseDao::backup(: SQL error:" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool SQLiteDatabaseDao::updateBackupPeriod(const QString &backupPeriodIntlId) noexcept
{
    QSqlQuery query;
    query.prepare(
        "update metadata "
        "set    backup_period_id = (select ebp.id"
        "                           from enum_backup_period ebp"
        "                           where ebp.sym_id = :sym_id"
        "                          );"
    );

    query.bindValue(":sym_id", backupPeriodIntlId);
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

bool SQLiteDatabaseDao::getMetadata(Metadata &metadata) const noexcept
{
    QSqlQuery query;
    query.setForwardOnly(true);

    const bool ok = query.exec(
        "select m.creation_date,"
        "       m.app_version,"
        "       m.last_optim_date,"
        "       m.last_backup_date,"
        "       m.next_backup_date,"
        "       m.backup_directory_path,"
        "       ebp.sym_id "
        "from metadata m "
        "join enum_backup_period ebp "
        "on m.backup_period_id = ebp.id;"
    );
    if (ok && query.next()) {
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
        metadata.backupPeriodSymId = query.value(6).toString();
    }
    return ok;
};

bool SQLiteDatabaseDao::getDatabaseVersion(Version &databaseVersion) const noexcept
{
    QSqlQuery query;
    const bool ok = query.exec("select m.app_version from metadata m;");
    if (ok && query.next()) {
        QString appVersion = query.value(0).toString();
        databaseVersion.fromString(appVersion);
    }
    return ok;
};

bool SQLiteDatabaseDao::getBackupDirectoryPath(QString &backupDirectoryPath) const noexcept
{
    QSqlQuery query;
    const bool ok = query.exec("select m.backup_directory_path from metadata m;");
    if (ok && query.next()) {
        backupDirectoryPath = query.value(0).toString();
    }
    return ok;
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
