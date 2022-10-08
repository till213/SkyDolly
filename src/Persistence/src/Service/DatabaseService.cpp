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

#include <QCoreApplication>
#include <QString>
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
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Settings.h>
#include <Kernel/Const.h>
#include <Model/Logbook.h>
#include <PersistenceManager.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/DatabaseDaoIntf.h"
#include <Service/DatabaseService.h>

namespace
{
    constexpr int BackupPeriodYearsNever = 999;
    constexpr int BackupPeriodOneMonth = 1;
    constexpr int BackupPeriodSevenDays = 7;
    constexpr int BackupPeriodOneDay = 1;
}

struct DatabaseServicePrivate
{
public:
    DatabaseServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          databaseDao(daoFactory->createDatabaseDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<DatabaseDaoIntf> databaseDao;
};

// PUBLIC

DatabaseService::DatabaseService() noexcept
    : d(std::make_unique<DatabaseServicePrivate>())
{
#ifdef DEBUG
    qDebug() << "DatabaseService::DatabaseService: CREATED";
#endif
}

DatabaseService::~DatabaseService() noexcept
{
#ifdef DEBUG
    qDebug() << "DatabaseService::~DatabaseService: DELETED";
#endif
}

bool DatabaseService::backup() noexcept
{
    QString backupDirectoryPath;

    PersistenceManager &persistenceManager = PersistenceManager::getInstance();
    bool ok {true};
    const Metadata metaData = persistenceManager.getMetadata(&ok);
    if (ok) {
        backupDirectoryPath = PersistenceManager::createBackupPathIfNotExists(metaData.backupDirectoryPath);
    }
    ok = !backupDirectoryPath.isNull();
    if (ok) {
        const QString backupFileName = persistenceManager.getBackupFileName(backupDirectoryPath);
        if (!backupFileName.isNull()) {
            const QString backupFilePath = backupDirectoryPath + "/" + backupFileName;
            ok = persistenceManager.backup(backupFilePath);
            if (ok) {
                ok = d->databaseDao->updateBackupDirectoryPath(backupDirectoryPath);
            }
        }
    }

    // Update the next backup date
    if (ok) {
        ok = updateBackupDate();
    }

    return ok;
}

bool DatabaseService::setBackupPeriod(const QString &backupPeriodIntlId) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->databaseDao->updateBackupPeriod(backupPeriodIntlId);
        if (ok) {
            ok = QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool DatabaseService::setNextBackupDate(const QDateTime &date) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->databaseDao->updateNextBackupDate(date);
        if (ok) {
            ok = QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool DatabaseService::updateBackupDate() noexcept
{
    bool ok {true};
    const Metadata metaData = PersistenceManager::getInstance().getMetadata(&ok);
    if (ok) {
        const QDateTime today = QDateTime::currentDateTime();
        QDateTime nextBackupDate = metaData.lastBackupDate.isNull() ? today : metaData.lastBackupDate;
        if (metaData.backupPeriodSymId == Const::BackupNeverSymId) {
            nextBackupDate = nextBackupDate.addYears(BackupPeriodYearsNever);
        } else if (metaData.backupPeriodSymId == Const::BackupMonthlySymId) {
            nextBackupDate = nextBackupDate.addMonths(BackupPeriodOneMonth);
        } else if (metaData.backupPeriodSymId == Const::BackupWeeklySymId) {
            nextBackupDate = nextBackupDate.addDays(BackupPeriodSevenDays);
        } else if (metaData.backupPeriodSymId == Const::BackupDailySymId) {
            nextBackupDate = nextBackupDate.addDays(BackupPeriodOneDay);
        }
        if (nextBackupDate < today) {
            nextBackupDate = today;
        }
        ok = setNextBackupDate(nextBackupDate);
    }
    return ok;
}

bool DatabaseService::setBackupDirectoryPath(const QString &backupDirectoryPath) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->databaseDao->updateBackupDirectoryPath(backupDirectoryPath);
        if (ok) {
            ok = QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

QString DatabaseService::getExistingLogbookPath(QWidget *parent) noexcept
{
    Settings &settings = Settings::getInstance();
    QString existingLogbookPath = QFileInfo(settings.getLogbookPath()).absolutePath();
    QString logbookPath = QFileDialog::getOpenFileName(parent, QCoreApplication::translate("DatabaseService", "Open Logbook"), existingLogbookPath, QString("*") % Const::LogbookExtension.data());
    return logbookPath;
}

QString DatabaseService::getNewLogbookPath(QWidget *parent) noexcept
{
    Settings &settings = Settings::getInstance();
    QString existingLogbookPath = settings.getLogbookPath();
    QFileInfo existingLogbookInfo(existingLogbookPath);
    QDir existingLogbookDirectory(existingLogbookInfo.absolutePath());
    if (existingLogbookDirectory.dirName() == existingLogbookInfo.baseName()) {
        existingLogbookDirectory.cdUp();
    }

    QString newLogbookPath;
    bool retry = true;
    while (retry) {
        QString logbookDirectoryPath = QFileDialog::getSaveFileName(parent, QCoreApplication::translate("DatabaseService", "New Logbook"), existingLogbookDirectory.absolutePath());
        if (!logbookDirectoryPath.isEmpty()) {
            QFileInfo info = QFileInfo(logbookDirectoryPath);
            if (!info.exists()) {
                newLogbookPath = logbookDirectoryPath + "/" % info.fileName() % Const::LogbookExtension.data();
                retry = false;
            } else {
                QMessageBox::information(parent, QCoreApplication::translate("DatabaseService", "Database exists"),
                                         QCoreApplication::translate("DatabaseService", "The logbook %1 already exists. Please choose another path.").arg(QDir::toNativeSeparators(logbookDirectoryPath)));
            }
        } else {
            retry = false;
        }
    }
    return newLogbookPath;
}
