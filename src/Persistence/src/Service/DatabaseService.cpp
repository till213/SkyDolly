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

#include <QWidget>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QDateTime>

#include "../../../Kernel/src/Settings.h"
#include "../../../Kernel/src/Const.h"
#include "../../../Model/src/Logbook.h"
#include "../ConnectionManager.h"
#include "../Dao/DaoFactory.h"
#include "../Dao/DatabaseDaoIntf.h"
#include "DatabaseService.h"

namespace
{
    constexpr int MaxBackupIndex = 1024;
}

class DatabaseServicePrivate
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
{}

DatabaseService::~DatabaseService() noexcept
{}

bool DatabaseService::backup() noexcept
{
    ConnectionManager &connectionManager = ConnectionManager::getInstance();
    const QString &logbookPath = connectionManager.getLogbookPath();
    QFileInfo logbookInfo = QFileInfo(logbookPath);

    const QString logbookDirectoryPath = logbookInfo.absolutePath();
    const QString baseName = logbookInfo.baseName();
    const QString backupDirectoryName = "Backups";
    QDir logbookDir(logbookDirectoryPath);
    bool ok;
    if (!logbookDir.exists(backupDirectoryName)) {
        ok = logbookDir.mkdir(backupDirectoryName);
    } else {
        ok = true;
    }
    if (ok) {
        const QString backupLogbookDirectoryPath = logbookDirectoryPath + "/" + backupDirectoryName;
        const QString baseBackupLogbookName = baseName + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmm");
        QString backupLogbookName = baseBackupLogbookName + Const::LogbookExtension;
        QDir backupLogbookDir(backupLogbookDirectoryPath);
        int index = 1;
        while (backupLogbookDir.exists(backupLogbookName) && index <= MaxBackupIndex) {
            backupLogbookName = baseBackupLogbookName + QString("-%1").arg(index) + Const::LogbookExtension;
            ++index;
        }
        ok = index <= MaxBackupIndex;
        if (ok) {
            const QString backupLogbookPath = backupLogbookDirectoryPath + "/" + backupLogbookName;
            ok = connectionManager.backup(backupLogbookPath);
        }
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

QString DatabaseService::getExistingLogbookPath(QWidget *parent) noexcept
{
    Settings &settings = Settings::getInstance();
    QString existingLogbookPath = QFileInfo(settings.getLogbookPath()).absolutePath();
    QString logbookPath = QFileDialog::getOpenFileName(parent, QT_TRANSLATE_NOOP("DatabaseService", "Open logbook"), existingLogbookPath, QString("*") + Const::LogbookExtension);
    return logbookPath;
}

QString DatabaseService::getNewLogbookPath(QWidget *parent) noexcept
{
    Settings &settings = Settings::getInstance();
    QString existingLogbookPath = settings.getLogbookPath();
    QString existingLogbookDirectoryPath = QFileInfo(existingLogbookPath).absolutePath();
    QString newLogbookPath;
    bool retry = true;
    while (retry) {
        QString logbookDirectoryPath = QFileDialog::getSaveFileName(parent, QT_TRANSLATE_NOOP("DatabaseService", "New logbook"), existingLogbookDirectoryPath);
        if (!logbookDirectoryPath.isEmpty()) {
            QFileInfo info = QFileInfo(logbookDirectoryPath);
            if (!info.exists()) {
                newLogbookPath = logbookDirectoryPath + "/" + info.fileName() + Const::LogbookExtension;
                retry = false;
            } else {
                QMessageBox::information(parent, QT_TRANSLATE_NOOP("DatabaseService", "Database exists"), QCoreApplication::translate("DatabaseService", "The logbook %1 already exists. Please choose another path.").arg(logbookDirectoryPath));
            }
        } else {
            retry = false;
        }
    }
    return newLogbookPath;
}
