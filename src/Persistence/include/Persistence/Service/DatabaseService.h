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
#ifndef DATABASESERVICE_H
#define DATABASESERVICE_H

#include <memory>
#include <cstdint>
#include <utility>
#include <cstdint>

#include <QObject>
#include <QSqlDatabase>

class QString;
class QWidget;
class QDateTime;

#include <Kernel/Const.h>
#include <Kernel/Version.h>
#include "../Connection.h"
#include "../Migration.h"
#include "../Metadata.h"
#include "../PersistenceLib.h"

class Version;
struct DatabaseServicePrivate;

class PERSISTENCE_API DatabaseService final
{
public:
    /*! The logbook backup mode. */
    enum struct BackupMode: std::uint8_t
    {
        /*! A logbook backup is to be created due to a migration of an older logbook; the next backup date is not updated. */
        Migration,
        /*! A periodic back is to be created; after backup the next backup date is updated. */
        Normal
    };

    /*! The logbook connection mode. */
    enum struct ConnectionMode: std::uint8_t
    {
        /*! The logbook is to be opened; a backup is made in case a migration is required (according to the application \e backup settings). */
        Open,
        /*! The logbook is to be imported; no backup is made in case a migration is necessary. */
        Import
    };

    DatabaseService(QString connectionName = Const::DefaultConnectionName) noexcept;
    DatabaseService(const DatabaseService &rhs) = delete;
    DatabaseService(DatabaseService &&rhs) noexcept;
    DatabaseService &operator=(const DatabaseService &rhs) = delete;
    DatabaseService &operator=(DatabaseService &&rhs) noexcept;
    ~DatabaseService();

    bool connect(const QString &logbookPath) noexcept;

    bool connectAndMigrate(const QString &logbookPath, ConnectionMode connectionMode, Migration::Milestones milestones = Migration::Milestone::All) noexcept;

    void disconnect(Connection::Default connection) noexcept;

    std::pair<bool, Version> checkDatabaseVersion() const noexcept;

    bool migrate(Migration::Milestones milestones = Migration::Milestone::All) noexcept;
    bool optimise() const noexcept;
    bool backup(const QString &logbookPath, BackupMode backupMode) noexcept;
    bool setBackupPeriod(std::int64_t backupPeriodId) noexcept;
    bool setNextBackupDate(const QDateTime &date) noexcept;
    bool updateBackupDate() noexcept;
    QString getBackupDirectoryPath(bool *ok = nullptr) const noexcept;
    bool setBackupDirectoryPath(const QString &backupFolderPath) noexcept;

    Metadata getMetadata(bool *ok = nullptr) const noexcept;
    Version getDatabaseVersion(bool *ok = nullptr) const noexcept;

    static QString getExistingLogbookPath(QWidget *parent) noexcept;
    static QString getNewLogbookPath(QWidget *parent) noexcept;
    static QString getBackupFileName(const QString &logbookPath, const QString &backupDirectoryPath) noexcept;
    static QString createBackupPathIfNotExists(const QString &logbookPath, const QString &relativeOrAbsoluteBackupDirectoryPath) noexcept;

private:
    std::unique_ptr<DatabaseServicePrivate> d;
};

#endif // DATABASESERVICE_H
