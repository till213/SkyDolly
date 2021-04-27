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
#include <QFileInfo>
#include <QDir>

#include "../../../Kernel/src/Settings.h"
#include "../ConnectionManager.h"
#include "DatabaseService.h"

namespace
{
    constexpr int MaxIndex = 1024;
}

// PUBLIC

DatabaseService::DatabaseService() noexcept
{}

DatabaseService::~DatabaseService() noexcept
{}

bool DatabaseService::connectDb() noexcept
{
    ConnectionManager &connectionManager = ConnectionManager::getInstance();
    const QString &libraryPath = Settings::getInstance().getLibraryPath();
    bool ok = connectionManager.connectDb(libraryPath);
    if (ok) {
        ok = connectionManager.migrate();
    }
    return ok;
}

void DatabaseService::disconnectDb() noexcept
{
    return ConnectionManager::getInstance().disconnectDb();
}

bool DatabaseService::isConnected() const noexcept
{
    return ConnectionManager::getInstance().isConnected();
}

const QString &DatabaseService::getLibraryPath() const noexcept
{
    return ConnectionManager::getInstance().getLibraryPath();
}

bool DatabaseService::optimise() noexcept
{
    return ConnectionManager::getInstance().optimise();
}

bool DatabaseService::backup() noexcept
{
    const QString &libraryPath = Settings::getInstance().getLibraryPath();
    QFileInfo libraryInfo = QFileInfo(libraryPath);

    const QString libraryDirectoryPath = libraryInfo.absolutePath();
    const QString baseName = libraryInfo.baseName();
    const QString backupDirectoryName = baseName + " Backups";
    QDir libraryDir(libraryDirectoryPath);
    bool ok;
    if (!libraryDir.exists(backupDirectoryName)) {
        ok = libraryDir.mkdir(backupDirectoryName);
    } else {
        ok = true;
    }
    if (ok) {
        const QString backupLibraryDirectoryPath = libraryDirectoryPath + "/" + backupDirectoryName;
        const QString baseBackupLibraryName = baseName + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmm");
        QString backupLibraryName = baseBackupLibraryName + LibraryExtension;
        QDir backupLibraryDir(backupLibraryDirectoryPath);
        int index = 1;
        while (backupLibraryDir.exists(backupLibraryName) && index <= MaxIndex) {
            backupLibraryName = baseBackupLibraryName + QString("-%1").arg(index) + LibraryExtension;
            ++index;
        }
        ok = index <= MaxIndex;
        if (ok) {
            const QString backupLibraryPath = backupLibraryDirectoryPath + "/" + backupLibraryName;
            ok = ConnectionManager::getInstance().backup(backupLibraryPath);
        }
    }
    return ok;
}

bool DatabaseService::getMetadata(Metadata &metadata) noexcept
{
    return ConnectionManager::getInstance().getMetadata(metadata);
}
