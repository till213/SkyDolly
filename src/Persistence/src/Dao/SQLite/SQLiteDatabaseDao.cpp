/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <utility>

#include <QString>
#include <QStringLiteral>
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
#include <Connection.h>
#include <Migration.h>
#include "SqlMigration.h"
#include "SQLiteDatabaseDao.h"

namespace
{
    constexpr const char *DriverName {"QSQLITE"};
}

struct DatabaseDaoPrivate
{
    DatabaseDaoPrivate(QString connectionName) noexcept
        : connectionName {std::move(connectionName)}
    {}

    QString connectionName;
};

// PUBLIC

SQLiteDatabaseDao::SQLiteDatabaseDao(QString connectionName) noexcept
    : d {std::make_unique<DatabaseDaoPrivate>(std::move(connectionName))}
{}

SQLiteDatabaseDao::SQLiteDatabaseDao(SQLiteDatabaseDao &&rhs) noexcept = default;
SQLiteDatabaseDao &SQLiteDatabaseDao::operator=(SQLiteDatabaseDao &&rhs) noexcept = default;

SQLiteDatabaseDao::~SQLiteDatabaseDao()
{
    disconnectSQLite(Connection::Default::Keep);
}

bool SQLiteDatabaseDao::connectDb(const QString &logbookPath) noexcept
{
    QSqlDatabase db = QSqlDatabase::addDatabase(::DriverName, d->connectionName);
    // For the QSQLITE driver, if the database name specified does not exist,
    // then it will create the file for you unless the QSQLITE_OPEN_READONLY
    // option is set
    db.setDatabaseName(logbookPath);
    return db.open();
}

void SQLiteDatabaseDao::disconnectDb(Connection::Default connection) noexcept
{
    disconnectSQLite(connection);
}

bool SQLiteDatabaseDao::migrate(Migration::Milestones milestones) const noexcept
{
    bool ok = createMigrationTable();
    if (ok) {
        SqlMigration sqlMigration {d->connectionName};
        ok = sqlMigration.migrate(milestones);
    }
    return ok;
}

bool SQLiteDatabaseDao::optimise() const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
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

bool SQLiteDatabaseDao::backup(const QString &backupPath) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    bool ok = query.exec(QStringLiteral("vacuum into '%1';").arg(backupPath));
    if (ok) {
        ok = query.exec("update metadata set last_backup_date = datetime('now') where rowid = 1;");
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteDatabaseDao::backup(: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }
    return ok;
}

bool SQLiteDatabaseDao::updateBackupPeriod(std::int64_t backupPeriodId) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update metadata "
        "set    backup_period_id = :backup_period_id;"
    );

    query.bindValue(":backup_period_id", QVariant::fromValue(backupPeriodId));
    return query.exec();
}

bool SQLiteDatabaseDao::updateNextBackupDate(const QDateTime &date) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.prepare(
        "update metadata "
        "set    next_backup_date = :next_backup_date;"
    );

    query.bindValue(":next_backup_date", date.toUTC());
    return query.exec();
}

bool SQLiteDatabaseDao::updateBackupDirectoryPath(const QString &backupDirectoryPath) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
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
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
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
        dateTime.setTimeZone(QTimeZone::UTC);
        metadata.creationDate = dateTime.toLocalTime();

        QString appVersion = query.value(1).toString();
        metadata.appVersion.fromString(appVersion);

        dateTime = query.value(2).toDateTime();
        dateTime.setTimeZone(QTimeZone::UTC);
        metadata.lastOptimisationDate = dateTime.toLocalTime();

        dateTime = query.value(3).toDateTime();
        dateTime.setTimeZone(QTimeZone::UTC);
        metadata.lastBackupDate = dateTime.toLocalTime();

        dateTime = query.value(4).toDateTime();
        dateTime.setTimeZone(QTimeZone::UTC);
        metadata.nextBackupDate = dateTime.toLocalTime();

        metadata.backupDirectoryPath = query.value(5).toString();
        metadata.backupPeriodId = query.value(6).toLongLong();
    }
    if (ok != nullptr) {
        *ok = success;
    }
#ifdef DEBUG
    if (!success) {
        qDebug() << "SQLiteDatabaseDao::getMetadata: SQL error" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
    }
#endif
    return metadata;
};

Version SQLiteDatabaseDao::getDatabaseVersion(bool *ok) const noexcept
{
    Version version;
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
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
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    const bool success = query.exec("select m.backup_directory_path from metadata m;");
    if (success && query.next()) {
        backupDirectoryPath = query.value(0).toString();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return backupDirectoryPath;
};

// PRIVATE

void SQLiteDatabaseDao::disconnectSQLite(Connection::Default connection) const noexcept
{
    // Make sure the 'db' instance goes out of scope before finally
    // removing the connection:
    // "Warning: There should be no open queries on the database connection
    // when this function is called, otherwise a resource leak will occur."
    {
        QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
        db.close();
    }
    if (d->connectionName != Const::DefaultConnectionName || connection == Connection::Default::Remove) {
        QSqlDatabase::removeDatabase(d->connectionName);
    }
}

bool SQLiteDatabaseDao::createMigrationTable() const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
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
