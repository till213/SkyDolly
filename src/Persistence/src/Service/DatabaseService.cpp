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

#include <QCoreApplication>
#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>
#include <QWidget>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QDateTime>

#include <Kernel/Settings.h>
#include <Kernel/Const.h>
#include <Model/Logbook.h>
#include <Service/EnumerationService.h>
#include <Service/DatabaseService.h>
#include <Migration.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/DatabaseDaoIntf.h"
#include "PersistedEnumerationItem.h"

namespace
{
    constexpr int MaxBackupIndex = 1024;

    constexpr int BackupPeriodYearsNever = 999;
    constexpr int BackupPeriodOneMonth = 1;
    constexpr int BackupPeriodSevenDays = 7;
    constexpr int BackupPeriodOneDay = 1;

    constexpr const char *DefaultBackupDirectory {"./Backups"};
}

struct DatabaseServicePrivate
{
    DatabaseServicePrivate(QString connectionName) noexcept
        :  connectionName(connectionName),
           daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite, std::move(connectionName))),
           databaseDao(daoFactory->createDatabaseDao())
    {}

    QString connectionName;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<DatabaseDaoIntf> databaseDao;

    const std::int64_t BackupPeriodNeverId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodNeverSymId).id()};
    const std::int64_t BackupPeriodNowId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodNowSymId).id()};
    const std::int64_t BackupPeriodMonthlyId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodMonthlySymId).id()};
    const std::int64_t BackupPeriodWeeklyId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodWeeklySymId).id()};
    const std::int64_t BackupPeriodDailyId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodDailySymId).id()};
    const std::int64_t BackupPeriodAlwaysId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodAlwaysSymId).id()};
};

// PUBLIC

DatabaseService::DatabaseService(QString connectionName) noexcept
    : d(std::make_unique<DatabaseServicePrivate>(std::move(connectionName)))
{}

DatabaseService::DatabaseService(DatabaseService &&rhs) noexcept = default;
DatabaseService &DatabaseService::operator=(DatabaseService &&rhs) noexcept = default;
DatabaseService::~DatabaseService() = default;

bool DatabaseService::connect(const QString &logbookPath) noexcept
{
    return d->databaseDao->connectDb(logbookPath);
}

bool DatabaseService::connectAndMigrate(const QString &logbookPath, ConnectionMode connectionMode, Migration::Milestones milestones) noexcept
{
    auto &settings = Settings::getInstance();
    bool ok = connect(logbookPath);
    if (ok) {
        const auto & [success, databaseVersion] = checkDatabaseVersion();
        ok = success;
        if (ok) {
            // Create a backup before migration of existing logbooks
            Version appVersion;
            // TODO: Check whether there are any migration steps to be executed at all before
            //       creating a backup, instead of comparing database and application versions
            //       (the later requires that the database version is always up to date)
            // For the time being we only compare major.minor but not major.minor.patch versions;
            // so we set the patch version always to 0
            Version refVersion {appVersion.getMajor(), appVersion.getMinor(), 0};
            if (connectionMode == ConnectionMode::Open && settings.isBackupBeforeMigrationEnabled() && !databaseVersion.isNull() && databaseVersion < refVersion) {
                ok = backup(logbookPath, DatabaseService::BackupMode::Migration);
            }
            if (ok) {
                // We still migrate, even if the above version check indicates that the database is up to date
                // (to make sure that we really do not miss any migration steps, in case the database version
                // was "forgotten" to be updated during some prior migration)
                ok = migrate(milestones);
            }
        }
    }
    return ok;
}

void DatabaseService::disconnect(Connection::Default connection) noexcept
{
    d->databaseDao->disconnectDb(connection);
}

std::pair<bool, Version> DatabaseService::checkDatabaseVersion() const noexcept
{
    std::pair<bool, Version> result;
    result.second = getDatabaseVersion(&result.first);
    if (result.first) {
        Version currentAppVersion;
        result.first = currentAppVersion >= result.second;
    } else {
        // New database - no metadata exists yet
        result.first = true;
        result.second = Version(0, 0, 0);
    }
    return result;
}

bool DatabaseService::migrate(Migration::Milestones milestones) noexcept
{
    return d->databaseDao->migrate(milestones);
}

bool DatabaseService::optimise() const noexcept
{
    return d->databaseDao->optimise();
}

bool DatabaseService::backup(const QString &logbookPath, BackupMode backupMode) noexcept
{
    bool ok {true};
    QString absoluteOrRelativeBackupPath = getBackupDirectoryPath(&ok);
    QString backupDirectoryPath;
    if (ok) {
        backupDirectoryPath = createBackupPathIfNotExists(logbookPath, absoluteOrRelativeBackupPath);
    }
    ok = !backupDirectoryPath.isNull();
    if (!ok && QFileInfo(absoluteOrRelativeBackupPath).isAbsolute()) {
        // Retry with a path relative to the logbook
        absoluteOrRelativeBackupPath = ::DefaultBackupDirectory;
        backupDirectoryPath = createBackupPathIfNotExists(logbookPath, absoluteOrRelativeBackupPath);
    }
    ok = !backupDirectoryPath.isNull();
    if (ok) {
        const QString backupFileName = getBackupFileName(logbookPath, backupDirectoryPath);
        if (!backupFileName.isNull()) {
            const QString backupFilePath = backupDirectoryPath + "/" + backupFileName;
            // No transaction must be active during backup
            ok = d->databaseDao->backup(backupFilePath);
            if (ok) {
                ok = setBackupDirectoryPath(backupDirectoryPath);
            }
        }
    }

    // Update the next backup date when not doing a migration
    if (ok && backupMode == BackupMode::Normal) {
        ok = updateBackupDate();
    }

    return ok;
}

bool DatabaseService::setBackupPeriod(std::int64_t backupPeriodId) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->databaseDao->updateBackupPeriod(backupPeriodId);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool DatabaseService::setNextBackupDate(const QDateTime &date) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->databaseDao->updateNextBackupDate(date);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
    }
    return ok;
}

bool DatabaseService::updateBackupDate() noexcept
{
    bool ok {true};
    const Metadata metaData = getMetadata(&ok);
    if (ok) {
        const QDateTime today = QDateTime::currentDateTime();
        QDateTime nextBackupDate = metaData.lastBackupDate.isNull() ? today : metaData.lastBackupDate;
        if (metaData.backupPeriodId == d->BackupPeriodNeverId) {
            nextBackupDate = nextBackupDate.addYears(BackupPeriodYearsNever);
        } else if (metaData.backupPeriodId == d->BackupPeriodMonthlyId) {
            nextBackupDate = nextBackupDate.addMonths(BackupPeriodOneMonth);
        } else if (metaData.backupPeriodId == d->BackupPeriodWeeklyId) {
            nextBackupDate = nextBackupDate.addDays(BackupPeriodSevenDays);
        } else if (metaData.backupPeriodId == d->BackupPeriodDailyId) {
            nextBackupDate = nextBackupDate.addDays(BackupPeriodOneDay);
        }
        if (nextBackupDate < today) {
            nextBackupDate = today;
        }
        ok = setNextBackupDate(nextBackupDate);
    }
    return ok;
}

QString DatabaseService::getBackupDirectoryPath(bool *ok) const noexcept
{
    QString backupDirectoryPath;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool success = db.transaction();
    if (success) {
        backupDirectoryPath = d->databaseDao->getBackupDirectoryPath(&success);
        db.rollback();
    }
    if (success && backupDirectoryPath.isNull()) {
        // Default backup location (in earlier logbooks the backup path was initially empty in the metadata)
        backupDirectoryPath = ::DefaultBackupDirectory;
    }
    if (ok != nullptr) {
        *ok = success;
    }    
    return backupDirectoryPath;
}

bool DatabaseService::setBackupDirectoryPath(const QString &backupDirectoryPath) noexcept
{
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool ok = db.transaction();
    if (ok) {
        ok = d->databaseDao->updateBackupDirectoryPath(backupDirectoryPath);
        if (ok) {
            ok = db.commit();
        } else {
            db.rollback();
        }
    }
    return ok;
}

Metadata DatabaseService::getMetadata(bool *ok) const noexcept
{
    Metadata metadata;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool success = db.transaction();
    if (success) {
        metadata = d->databaseDao->getMetadata(&success);
        db.rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return metadata;
}

Version DatabaseService::getDatabaseVersion(bool *ok) const noexcept
{
    Version version;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    bool success = db.transaction();
    if (success) {
        version = d->databaseDao->getDatabaseVersion(&success);
        db.rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return version;
}

QString DatabaseService::getExistingLogbookPath(QWidget *parent) noexcept
{
    const auto &settings = Settings::getInstance();
    const QString existingLogbookPath = QFileInfo(settings.getLogbookPath()).absolutePath();
    const QString logbookPath = QFileDialog::getOpenFileName(parent, QCoreApplication::translate("DatabaseService", "Open Logbook"), existingLogbookPath, QStringLiteral("*") % Const::DotLogbookExtension);
    return logbookPath;
}

QString DatabaseService::getNewLogbookPath(QWidget *parent) noexcept
{
    const auto &settings = Settings::getInstance();
    const QString existingLogbookPath = settings.getLogbookPath();
    const QFileInfo existingLogbookInfo {existingLogbookPath};
    QDir existingLogbookDirectory(existingLogbookInfo.absolutePath());
    if (existingLogbookDirectory.dirName() == existingLogbookInfo.baseName()) {
        existingLogbookDirectory.cdUp();
    }

    QString newLogbookPath;
    bool retry = true;
    while (retry) {
        QString logbookDirectoryPath = QFileDialog::getSaveFileName(parent, QCoreApplication::translate("DatabaseService", "New Logbook"), existingLogbookDirectory.absolutePath());
        if (!logbookDirectoryPath.isEmpty()) {
            const QFileInfo fileInfo = QFileInfo(logbookDirectoryPath);
            if (!fileInfo.exists()) {
                newLogbookPath = logbookDirectoryPath + "/" % fileInfo.fileName() % Const::DotLogbookExtension;
                retry = false;
            } else {
                QMessageBox::information(parent, QCoreApplication::translate("DatabaseService", "Database Exists"),
                                         QCoreApplication::translate("DatabaseService",
                                         "The logbook %1 already exists. Please choose another path.").arg(QDir::toNativeSeparators(logbookDirectoryPath)));
            }
        } else {
            retry = false;
        }
    }
    return newLogbookPath;
}

QString DatabaseService::getBackupFileName(const QString &logbookPath, const QString &backupDirectoryPath) noexcept
{
    const QDir backupDir {backupDirectoryPath};

    const QFileInfo logbookInfo = QFileInfo(logbookPath);
    const QString baseName = logbookInfo.completeBaseName();
    const QString baseBackupLogbookName = baseName + "-" + QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hhmm"));
    QString backupLogbookName = baseBackupLogbookName % Const::DotLogbookExtension;
    int index = 1;
    while (backupDir.exists(backupLogbookName) && index <= ::MaxBackupIndex) {
        backupLogbookName = baseBackupLogbookName % QStringLiteral("-%1").arg(index) % Const::DotLogbookExtension;
        ++index;
    }
    return (index <= ::MaxBackupIndex) ? backupLogbookName : QString();
}

QString DatabaseService::createBackupPathIfNotExists(const QString &logbookPath, const QString &relativeOrAbsoluteBackupDirectoryPath) noexcept
{
    QString existingBackupPath;
    if (QDir::isRelativePath(relativeOrAbsoluteBackupDirectoryPath)) {
        const QString &logbookDirectoryPath = QFileInfo(logbookPath).absolutePath();
        existingBackupPath = logbookDirectoryPath + "/" + QFileInfo(relativeOrAbsoluteBackupDirectoryPath).fileName();
    } else {
        existingBackupPath = relativeOrAbsoluteBackupDirectoryPath;
    }

    const QDir backupDir(existingBackupPath);
    if (!backupDir.exists()) {
         const bool ok = backupDir.mkpath(existingBackupPath);
         if (!ok) {
             existingBackupPath.clear();
         }
    }
    return existingBackupPath;
}
