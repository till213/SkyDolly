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

#include "../../../Kernel/src/Settings.h"
#include "../../../Kernel/src/Const.h"
#include "../../../Model/src/Logbook.h"
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

bool DatabaseService::connectDb() noexcept
{
    ConnectionManager &connectionManager = ConnectionManager::getInstance();
    const QString &logbookPath = Settings::getInstance().getLogbookPath();
    const QString logbookDirectoryPath = QFileInfo(logbookPath).absolutePath();
    QFileInfo info(logbookDirectoryPath);
    bool ok = info.exists();
    if (!ok) {
        QDir dir(logbookDirectoryPath);
        ok = dir.mkpath(logbookDirectoryPath);
    }
    if (ok) {
        ok = connectionManager.connectDb(logbookPath);
        if (ok) {
            Flight &flight = Logbook::getInstance().getCurrentFlight();
            flight.clear();
            ok = connectionManager.migrate();
        }
    }
    emit connectionStateChanged(ok);
    return ok;
}

void DatabaseService::disconnectDb() noexcept
{
    ConnectionManager::getInstance().disconnectDb();
    emit connectionStateChanged(false);
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

bool DatabaseService::getMetadata(Metadata &metadata) noexcept
{
    return ConnectionManager::getInstance().getMetadata(metadata);
}
