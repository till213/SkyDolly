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
#include <QObject>
#include <QFileInfo>
#include <QDir>
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QCoreApplication>
#include <QTimer>

#include "../../../Kernel/src/Settings.h"
#include "../../../Kernel/src/Const.h"
#include "../ConnectionManager.h"
#include "DatabaseService.h"

namespace
{
    constexpr int MaxBackupIndex = 1024;
}

// PUBLIC

DatabaseService::DatabaseService(QObject *parent) noexcept
    : QObject(parent)
{}

DatabaseService::~DatabaseService() noexcept
{}

bool DatabaseService::connectWithLogbook(const QString &logbookPath, QWidget *parent) noexcept
{
    ConnectionManager &connectionManager = ConnectionManager::getInstance();
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
            ok = connectionManager.connectDb(currentLogbookPath);
            if (ok) {
                Version databaseVersion;
                ok = checkDatabaseVersion(databaseVersion);
                if (ok) {
                    ok = connectionManager.migrate();
                    if (ok) {
                        Settings::getInstance().setLogbookPath(currentLogbookPath);
                    }
                    retry = false;
                } else {
                    disconnectFromLogbook();
                    QMessageBox messageBox(parent);
                    messageBox.setWindowIcon(QIcon(":/img/icons/application-icon.png"));
                    messageBox.setText(tr("The logbook %1 has been created with a newer version %2.").arg(currentLogbookPath, databaseVersion.toString()));
                    messageBox.setInformativeText("Do you want to create a new logbook?");
                    QPushButton *createNewPushButton = messageBox.addButton(tr("Create new logbook"), QMessageBox::AcceptRole);
                    QPushButton *openExistingPushButton = messageBox.addButton(tr("Open another logbook"), QMessageBox::AcceptRole);
                    messageBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
                    messageBox.setDefaultButton(createNewPushButton);
                    messageBox.setIcon(QMessageBox::Icon::Question);

                    messageBox.exec();
                    const QAbstractButton *clickedButton = messageBox.clickedButton();
                    if (clickedButton == createNewPushButton) {
                        currentLogbookPath = getNewLogbookPath(nullptr);
                    } else if (clickedButton == openExistingPushButton) {
                        currentLogbookPath = getExistingLogbookPath(nullptr);
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
    emit logbookConnectionChanged(ok);
    return ok;
}

void DatabaseService::disconnectFromLogbook() noexcept
{
    ConnectionManager::getInstance().disconnectDb();
    emit logbookConnectionChanged(false);
}

bool DatabaseService::isConnected() const noexcept
{
    return ConnectionManager::getInstance().isConnected();
}

const QString &DatabaseService::getLogbookPath() const noexcept
{
    return ConnectionManager::getInstance().getLogbookPath();
}

bool DatabaseService::optimise() noexcept
{
    return ConnectionManager::getInstance().optimise();
}

bool DatabaseService::backup() noexcept
{
    const QString &logbookPath = getLogbookPath();
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
            ok = ConnectionManager::getInstance().backup(backupLogbookPath);
        }
    }
    return ok;
}

bool DatabaseService::getMetadata(Metadata &metadata) const noexcept
{
    return ConnectionManager::getInstance().getMetadata(metadata);
}

QString DatabaseService::getExistingLogbookPath(QWidget *parent) noexcept
{
    Settings &settings = Settings::getInstance();
    QString existingLogbookPath = QFileInfo(settings.getLogbookPath()).absolutePath();
    QString logbookPath = QFileDialog::getOpenFileName(parent, tr("Open logbook"), existingLogbookPath, QString("*") + Const::LogbookExtension);
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
        QString logbookDirectoryPath = QFileDialog::getSaveFileName(parent, tr("New logbook"), existingLogbookDirectoryPath);
        if (!logbookDirectoryPath.isEmpty()) {
            QFileInfo info = QFileInfo(logbookDirectoryPath);
            if (!info.exists()) {
                newLogbookPath = logbookDirectoryPath + "/" + info.fileName() + Const::LogbookExtension;
                retry = false;
            } else {
                QMessageBox::information(parent, tr("Database exists"), tr("The logbook %1 already exists. Please choose another path.").arg(logbookDirectoryPath));
            }
        } else {
            retry = false;
        }
    }
    return newLogbookPath;
}

// PRIVATE

bool DatabaseService::checkDatabaseVersion(Version &databaseVersion) const noexcept
{
    Version appVersion;
    Metadata metadata;
    bool ok = getMetadata(metadata);
    if (ok) {
        ok = appVersion >= metadata.appVersion;
        databaseVersion = metadata.appVersion;
    } else {
        // New database - no metadata exists yet
        ok = true;
        databaseVersion = Version(0, 0, 0);
    }
    return ok;
}
