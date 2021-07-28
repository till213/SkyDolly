/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QSqlDatabase>

#include "../../Kernel/src/Const.h"
#include "../../Kernel/src/Settings.h"
#include "../../Model/src/Logbook.h"
#include "../../Model/src/Flight.h"
#include "../../Kernel/src/Version.h"
#include "Metadata.h"
#include "Dao/DaoFactory.h"
#include "Dao/DatabaseDaoIntf.h"
#include "Service/DatabaseService.h"
#include "ConnectionManager.h"

class ConnectionManagerPrivate
{
public:
    ConnectionManagerPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          databaseDao(daoFactory->createDatabaseDao()),
          connected(false)
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<DatabaseDaoIntf> databaseDao;
    QString logbookPath;
    bool connected;

    static ConnectionManager *instance; 
};

ConnectionManager *ConnectionManagerPrivate::instance = nullptr;

// PUBLIC

ConnectionManager &ConnectionManager::getInstance() noexcept
{
    if (ConnectionManagerPrivate::instance == nullptr) {
        ConnectionManagerPrivate::instance = new ConnectionManager();
    }
    return *ConnectionManagerPrivate::instance;
}

void ConnectionManager::destroyInstance() noexcept
{
    if (ConnectionManagerPrivate::instance != nullptr) {
        delete ConnectionManagerPrivate::instance;
        ConnectionManagerPrivate::instance = nullptr;
    }
}

bool ConnectionManager::connectWithLogbook(const QString &logbookPath, QWidget *parent) noexcept
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
                Version databaseVersion;
                ok = checkDatabaseVersion(databaseVersion);
                if (ok) {
                    Flight &flight = Logbook::getInstance().getCurrentFlight();
                    flight.clear(true);
                    ok = migrate();
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

void ConnectionManager::disconnectFromLogbook() noexcept
{
    d->databaseDao->disconnectDb();
    d->logbookPath.clear();
    d->connected = false;
    emit connectionChanged(d->connected);
}

bool ConnectionManager::isConnected() const noexcept
{
    return d->connected;
}

const QString &ConnectionManager::getLogbookPath() const noexcept
{
    return d->logbookPath;
}

bool ConnectionManager::migrate() noexcept
{
    return d->databaseDao->migrate();
}

bool ConnectionManager::optimise() noexcept
{
    return d->databaseDao->optimise();
}

bool ConnectionManager::backup(const QString &backupLogbookPath) noexcept
{
    return d->databaseDao->backup(backupLogbookPath);
}

bool ConnectionManager::getMetadata(Metadata &metadata) const noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->databaseDao->getMetadata(metadata);
        QSqlDatabase::database().rollback();
    }
    return ok;
}

bool ConnectionManager::getDatabaseVersion(Version &databaseVersion) const noexcept
{
    return d->databaseDao->getDatabaseVersion(databaseVersion);
}

// PROTECTED

ConnectionManager::~ConnectionManager() noexcept
{
    disconnectFromLogbook();
#ifdef DEBUG
    qDebug("ConnectionManager::ConnectionManager: DELETED");
#endif
}

// PRIVATE

ConnectionManager::ConnectionManager() noexcept
    : QObject(),
      d(std::make_unique<ConnectionManagerPrivate>())
{
#ifdef DEBUG
    qDebug("ConnectionManager::ConnectionManager: CREATED");
#endif
}

bool ConnectionManager::connectDb(const QString &logbookPath) noexcept
{
    bool ok;
    if (d->logbookPath != logbookPath) {
        ok = d->databaseDao->connectDb(logbookPath);
        d->logbookPath = logbookPath;
    } else {
        ok = false;
    }
    return ok;
}

bool ConnectionManager::checkDatabaseVersion(Version &databaseVersion) const noexcept
{
    Version currentAppVersion;
    bool ok = getDatabaseVersion(databaseVersion);
    if (ok) {
        ok = currentAppVersion >= databaseVersion;
    } else {
        // New database - no metadata exists yet
        ok = true;
        databaseVersion = Version(0, 0, 0);
    }
    return ok;
}
