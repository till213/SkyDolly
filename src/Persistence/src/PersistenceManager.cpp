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
#include <QStringBuilder>
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
#include "Service/DatabaseService.h"
#include "PersistenceManager.h"

struct PersistenceManagerPrivate
{
    std::unique_ptr<DatabaseService> databaseService {std::make_unique<DatabaseService>()};
    QString logbookPath;
    bool connected {false};

    static inline std::once_flag onceFlag;
    static inline std::unique_ptr<PersistenceManager> instance;
};

// PUBLIC

PersistenceManager &PersistenceManager::getInstance() noexcept
{
    std::call_once(PersistenceManagerPrivate::onceFlag, []() {
        PersistenceManagerPrivate::instance = std::unique_ptr<PersistenceManager>(new PersistenceManager());
    });
    return *PersistenceManagerPrivate::instance;
}

void PersistenceManager::destroyInstance() noexcept
{
    if (PersistenceManagerPrivate::instance != nullptr) {
        PersistenceManagerPrivate::instance.reset();
    }
}

bool PersistenceManager::connectWithLogbook(const QString &logbookPath, QWidget *parent) noexcept
{
    bool ok {true};
    auto &settings = Settings::getInstance();
    QString selectedLogbookPath = logbookPath;
    bool retry {true};
    while (retry && ok) {
        const QString logbookDirectoryPath = QFileInfo(selectedLogbookPath).absolutePath();
        const QFileInfo fileInfo(logbookDirectoryPath);
        ok = fileInfo.exists();
        if (!ok) {
            const QDir dir {logbookDirectoryPath};
            ok = dir.mkpath(logbookDirectoryPath);
        }
        if (ok) {
            if (isConnected()) {
                disconnectFromLogbook();
            }
            ok = d->databaseService->connect(selectedLogbookPath);
            if (ok) {
                const auto & [success, databaseVersion] = d->databaseService->checkDatabaseVersion();
                ok = success;
                if (ok) {
                    auto &flight = Logbook::getInstance().getCurrentFlight();
                    flight.clear(true, FlightData::CreationTimeMode::Reset);
                    // Create a backup before migration of existing logbooks
                    Version appVersion;
                    // TODO: Check whether there are any migration steps to be executed at all before
                    //       creating a backup, instead of comparing database and application versions
                    //       (the later requires that the database version is always up to date)
                    // For the time being we only compare major.minor but not major.minor.patch versions;
                    // so we set the patch version always to 0
                    Version refVersion {appVersion.getMajor(), appVersion.getMinor(), 0};
                    if (!databaseVersion.isNull() && settings.isBackupBeforeMigrationEnabled() && databaseVersion < refVersion) {
                        ok = d->databaseService->backup(selectedLogbookPath, DatabaseService::BackupMode::Migration);
                    }
                    if (ok) {
                        // We still migrate, even if the above version check indicates that the database is up to date
                        // (to make sure that we really do not miss any migration steps, in case the database version
                        // was "forgotten" to be updated during some prior migration)
                        ok = d->databaseService->migrate();
                    }
                    retry = false;
                } else {
                    disconnectFromLogbook();
                    std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(parent);
                    messageBox->setWindowIcon(QIcon(":/img/icons/application-icon.png"));
                    messageBox->setWindowTitle(tr("Newer Version"));
                    messageBox->setText(tr("The logbook %1 has been created with a newer version %2. Do you want to create a new logbook?").arg(QDir::toNativeSeparators(selectedLogbookPath), databaseVersion.toString()));
                    messageBox->setInformativeText(tr("Logbooks created with newer %1 versions cannot be opened.").arg(Version::getApplicationName()));
                    QPushButton *createNewPushButton = messageBox->addButton(tr("Create &New Logbook"), QMessageBox::AcceptRole);
                    QPushButton *openExistingPushButton = messageBox->addButton(tr("&Open Another Logbook"), QMessageBox::AcceptRole);
                    messageBox->addButton(tr("&Cancel"), QMessageBox::RejectRole);
                    messageBox->setDefaultButton(createNewPushButton);
                    messageBox->setIcon(QMessageBox::Icon::Question);

                    messageBox->exec();
                    const QAbstractButton *clickedButton = messageBox->clickedButton();
                    if (clickedButton == createNewPushButton) {
                        selectedLogbookPath = DatabaseService::getNewLogbookPath(parent);
                    } else if (clickedButton == openExistingPushButton) {
                        selectedLogbookPath = DatabaseService::getExistingLogbookPath(parent);
                    } else {
                        selectedLogbookPath.clear();
                    }
                    if (!selectedLogbookPath.isNull()) {
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
        d->logbookPath = selectedLogbookPath;
        settings.setLogbookPath(d->logbookPath);
        emit connectionChanged(true);
    } else {
        disconnectFromLogbook();
    }

    return ok;
}

void PersistenceManager::disconnectFromLogbook() noexcept
{
    d->databaseService->disconnect(Connection::Default::Remove);
    d->logbookPath.clear();
    d->connected = false;
    emit connectionChanged(d->connected);
}

bool PersistenceManager::isConnected() const noexcept
{
    return d->connected;
}

QString PersistenceManager::getLogbookPath() const noexcept
{
    return d->logbookPath;
}

bool PersistenceManager::optimise() const noexcept
{
    return d->databaseService->optimise();
}

Metadata PersistenceManager::getMetadata(bool *ok) const noexcept
{
    return d->databaseService->getMetadata(ok);
}

Version PersistenceManager::getDatabaseVersion(bool *ok) const noexcept
{
    return d->databaseService->getDatabaseVersion(ok);
}

QString PersistenceManager::getBackupDirectoryPath(bool *ok) const noexcept
{
    return d->databaseService->getBackupDirectoryPath(ok);
}

// PRIVATE

PersistenceManager::PersistenceManager() noexcept
    : QObject(),
      d {std::make_unique<PersistenceManagerPrivate>()}
{}

PersistenceManager::~PersistenceManager()
{
#ifdef DEBUG
    qDebug() << "PersistenceManager::~PersistenceManager: DELETED";
#endif
    disconnectFromLogbook();
}
