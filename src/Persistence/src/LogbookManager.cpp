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
#include <mutex>

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QSqlDatabase>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Const.h>
#include <Kernel/Settings.h>
#include <Kernel/Version.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include "Metadata.h"
#include "Dao/DaoFactory.h"
#include "Dao/DatabaseDaoIntf.h"
#include "Service/DatabaseService.h"
#include "LogbookManager.h"

namespace {
    constexpr int MaxBackupIndex = 1024;
}

struct LogbookManagerPrivate
{
    LogbookManagerPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          databaseDao(daoFactory->createDatabaseDao()),
          connected(false)
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<DatabaseDaoIntf> databaseDao;
    QString logbookPath;
    bool connected;

    static inline std::once_flag onceFlag;
    static inline LogbookManager *instance;
};


// PUBLIC

LogbookManager &LogbookManager::getInstance() noexcept
{
    std::call_once(LogbookManagerPrivate::onceFlag, []() {
        LogbookManagerPrivate::instance = new LogbookManager();
    });
    return *LogbookManagerPrivate::instance;
}

void LogbookManager::destroyInstance() noexcept
{
    if (LogbookManagerPrivate::instance != nullptr) {
        delete LogbookManagerPrivate::instance;
        LogbookManagerPrivate::instance = nullptr;
    }
}

bool LogbookManager::connectWithLogbook(const QString &logbookPath, QWidget *parent) noexcept
{
    QString currentLogbookPath = logbookPath;
    bool ok = true;
    bool retry = true;
    while (retry && ok) {
        const QString logbookDirectoryPath = QFileInfo(currentLogbookPath).absolutePath();
        QFileInfo info(logbookDirectoryPath);
        ok = info.exists();
        if (!ok) {
            QDir dir(logbookDirectoryPath);
            ok = dir.mkpath(logbookDirectoryPath);
        }
        if (ok) {
            if (isConnected()) {
                disconnectFromLogbook();
            }
            ok = connectDb(currentLogbookPath);
            if (ok) {
                const auto & [success, databaseVersion] = checkDatabaseVersion();
                ok = success;
                if (ok) {
                    Settings &settings = Settings::getInstance();
                    Flight &flight = Logbook::getInstance().getCurrentFlight();
                    flight.clear(true);
                    // Create a backup before migration of existing logbooks
                    Version appVersion;
                    if (!databaseVersion.isNull() && settings.isBackupBeforeMigrationEnabled() && databaseVersion < appVersion) {
                        QString backupDirectoryPath = d->databaseDao->getBackupDirectoryPath(&ok);
                        if (ok) {
                            if (backupDirectoryPath.isNull()) {
                                // Default backup location, relative to logbook path
                                backupDirectoryPath = "./Backups";
                            }
                            backupDirectoryPath = createBackupPathIfNotExists(backupDirectoryPath);
                            ok = !backupDirectoryPath.isNull();
                        }
                        QString backupFileName;
                        if (ok) {
                            backupFileName = getBackupFileName(backupDirectoryPath);
                            ok = !backupFileName.isNull();
                        }
                        if (ok) {
                            ok = backup(backupDirectoryPath + "/" + backupFileName);
                        }
                    }
                    if (ok) {
                        // We still migrate, even if the above version check indicates that the database is up to date
                        // (to make sure that we really do not miss any migration steps, in case the database version
                        // was "forgotten" to be updated during some prior migration)
                        ok = migrate();
                    }
                    if (ok) {
                        settings.setLogbookPath(currentLogbookPath);
                    }
                    retry = false;
                } else {
                    disconnectFromLogbook();
                    std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(parent);
                    messageBox->setWindowIcon(QIcon(":/img/icons/application-icon.png"));
                    messageBox->setWindowTitle(tr("Newer Version"));
                    messageBox->setText(tr("The logbook %1 has been created with a newer version %2. Do you want to create a new logbook?").arg(QDir::toNativeSeparators(currentLogbookPath), databaseVersion.toString()));
                    messageBox->setInformativeText(tr("Logbooks created with newer %1 versions cannot be opened.").arg(Version::getApplicationName()));
                    QPushButton *createNewPushButton = messageBox->addButton(tr("Create &New Logbook"), QMessageBox::AcceptRole);
                    QPushButton *openExistingPushButton = messageBox->addButton(tr("&Open Another Logbook"), QMessageBox::AcceptRole);
                    messageBox->addButton(tr("&Cancel"), QMessageBox::RejectRole);
                    messageBox->setDefaultButton(createNewPushButton);
                    messageBox->setIcon(QMessageBox::Icon::Question);

                    messageBox->exec();
                    const QAbstractButton *clickedButton = messageBox->clickedButton();
                    if (clickedButton == createNewPushButton) {
                        currentLogbookPath = DatabaseService::getNewLogbookPath(parent);
                    } else if (clickedButton == openExistingPushButton) {
                        currentLogbookPath = DatabaseService::getExistingLogbookPath(parent);
                    } else {
                        currentLogbookPath.clear();
                    }
                    if (!currentLogbookPath.isNull()) {
                        retry = true;
                        ok = true;
                    } else {
                        retry = false;
                        ok = false;
                    }
                }
            }
        }
    }
    d->connected = ok;
    if (d->connected) {
        emit connectionChanged(true);
    } else {
        disconnectFromLogbook();
    }

    return ok;
}

void LogbookManager::disconnectFromLogbook() noexcept
{
    d->databaseDao->disconnectDb();
    d->logbookPath.clear();
    d->connected = false;
    emit connectionChanged(d->connected);
}

bool LogbookManager::isConnected() const noexcept
{
    return d->connected;
}

const QString &LogbookManager::getLogbookPath() const noexcept
{
    return d->logbookPath;
}

bool LogbookManager::migrate() noexcept
{
    return d->databaseDao->migrate();
}

bool LogbookManager::optimise() noexcept
{
    return d->databaseDao->optimise();
}

bool LogbookManager::backup(const QString &backupLogbookPath) noexcept
{
    return d->databaseDao->backup(backupLogbookPath);
}

Metadata LogbookManager::getMetadata(bool *ok) const noexcept
{
    Metadata metadata;
    bool success = QSqlDatabase::database().transaction();
    if (success) {
        metadata = d->databaseDao->getMetadata(&success);
        QSqlDatabase::database().rollback();
    }
    if (ok != nullptr) {
        *ok = success;
    }
    return metadata;
}

Version LogbookManager::getDatabaseVersion(bool *ok) const noexcept
{
    return d->databaseDao->getDatabaseVersion(ok);
}

QString LogbookManager::getBackupDirectoryPath(bool *ok) const noexcept
{
    return d->databaseDao->getBackupDirectoryPath(ok);
}

QString LogbookManager::getBackupFileName(const QString &backupDirectoryPath) const noexcept
{
    QDir backupDir(backupDirectoryPath);
    const QString &logbookPath = getLogbookPath();
    const QFileInfo logbookInfo = QFileInfo(logbookPath);
    const QString baseName = logbookInfo.completeBaseName();
    const QString baseBackupLogbookName = baseName + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmm");
    QString backupLogbookName = baseBackupLogbookName + Const::LogbookExtension;
    int index = 1;
    while (backupDir.exists(backupLogbookName) && index <= MaxBackupIndex) {
        backupLogbookName = baseBackupLogbookName + QString("-%1").arg(index) + Const::LogbookExtension;
        ++index;
    }
    if (index <= MaxBackupIndex) {
        return backupLogbookName;
    } else {
        return QString();
    }
}

QString LogbookManager::createBackupPathIfNotExists(const QString &relativeOrAbsoluteBackupDirectoryPath) noexcept
{
    QString existingBackupPath;
    if (QDir::isRelativePath(relativeOrAbsoluteBackupDirectoryPath)) {
        const LogbookManager &LogbookManager = LogbookManager::getInstance();
        const QString &logbookDirectoryPath = QFileInfo(LogbookManager.getLogbookPath()).absolutePath();
        existingBackupPath = logbookDirectoryPath + "/" + QFileInfo(relativeOrAbsoluteBackupDirectoryPath).fileName();
    } else {
        existingBackupPath = relativeOrAbsoluteBackupDirectoryPath;
    }

    QDir backupDir(existingBackupPath);
    if (!backupDir.exists()) {
         const bool ok = backupDir.mkpath(existingBackupPath);
         if (!ok) {
             existingBackupPath.clear();
         }
    }
    return existingBackupPath;
}

// PROTECTED

LogbookManager::~LogbookManager() noexcept
{
    disconnectFromLogbook();
#ifdef DEBUG
    qDebug() << "LogbookManager::~LogbookManager: DELETED";
#endif
}

// PRIVATE

LogbookManager::LogbookManager() noexcept
    : QObject(),
      d(std::make_unique<LogbookManagerPrivate>())
{
#ifdef DEBUG
    qDebug() << "LogbookManager::LogbookManager: CREATED";
#endif
}

bool LogbookManager::connectDb(const QString &logbookPath) noexcept
{
    bool ok {true};
    if (d->logbookPath != logbookPath) {
        ok = d->databaseDao->connectDb(logbookPath);
        d->logbookPath = logbookPath;
    }
    return ok;
}

std::pair<bool, Version>  LogbookManager::checkDatabaseVersion() const noexcept
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
