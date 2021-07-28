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

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QTimeZone>

#include "../../../../Kernel/src/Settings.h"
#include "../../../../Kernel/src/Version.h"
#include "../../Metadata.h"
#include "SqlMigration.h"
#include "SQLiteDatabaseDao.h"

constexpr char DbName[] = "QSQLITE";

class DatabaseDaoPrivate
{
public:
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
        ok = sqlMigration.migrateExAnte();
        if (ok) {
            ok = sqlMigration.migrateDdl();
        }
        if (ok) {
            ok = sqlMigration.migrateExPost();
        }
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
        qDebug("SQLiteDatabaseDao::optimise(: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
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
        qDebug("SQLiteDatabaseDao::backup(: SQL error: %s", qPrintable(query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode()));
#endif
    }
    return ok;
}

bool SQLiteDatabaseDao::getMetadata(Metadata &metadata) const noexcept
{
    QSqlQuery query;
    bool ok = query.exec(
"select m.creation_date, m.app_version, m.last_optim_date, m.last_backup_date, m.backup_directory_path, e.intl_id "
"from metadata m "
"left join enum_backup_interval e "
"on m.backup_interval_id = e.id;"

);
    if (query.next()) {
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

        metadata.backupDirectoryPath = query.value(4).toString();
        metadata.backupPeriodIntlId = query.value(5).toString();
    }
    return ok;
};

bool SQLiteDatabaseDao::getDatabaseVersion(Version &databaseVersion) const noexcept
{
    QSqlQuery query;
    bool ok = query.exec("select m.app_version from metadata m;");
    if (query.next()) {
        QString appVersion = query.value(0).toString();
        databaseVersion.fromString(appVersion);
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
    query.prepare("create table if not exists migr("
    "id text not null,"
    "step integer not null,"
    "success integer not null,"
    "timestamp datetime default current_timestamp,"
    "msg text,"
    "primary key (id, step));");
    return query.exec();
}
