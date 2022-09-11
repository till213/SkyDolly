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
#ifndef LOGBOOKMANAGER_H
#define LOGBOOKMANAGER_H

#include <memory>
#include <utility>

#include <QObject>

class QString;
class QWidget;

class Metadata;
class Version;

#include "PersistenceLib.h"

class Metadata;
class Version;
struct LogbookManagerPrivate;

class PERSISTENCE_API LogbookManager : public QObject
{
    Q_OBJECT
public:
    static LogbookManager &getInstance() noexcept;
    static void destroyInstance() noexcept;

    /*!
     * Connects with the database given by \c logbookPath
     * and initialises the database by applying the required migrations.
     *
     * If a problem with opening the database occurs (e.g. a version
     * mismatch) then a dialog asks the user for alternative logbook
     * paths (or to quit the application altogether).
     *
     * The actual logbook path (which is usually the given \c logbookPath)
     * is stored in the Settings.
     *
     * \param logbookPath
     *        the path of the logbook (database) file to connect with
     * \return \c true if the connection succeeded; \c false else
     * \sa Settings#setLogbookPath
     * \sa connectionChanged
     */
    bool connectWithLogbook(const QString &logbookPath, QWidget *parent) noexcept;

    void disconnectFromLogbook() noexcept;
    bool isConnected() const noexcept;
    const QString &getLogbookPath() const noexcept;

    bool migrate() noexcept;
    bool optimise() noexcept;
    bool backup(const QString &backupLogbookPath) noexcept;

    Metadata getMetadata(bool *ok = nullptr) const noexcept;
    Version getDatabaseVersion(bool *ok = nullptr) const noexcept;
    QString getBackupDirectoryPath(bool *ok = nullptr) const noexcept;

    QString getBackupFileName(const QString &backupDirectoryPath) const noexcept;
    static QString createBackupPathIfNotExists(const QString &relativeOrAbsoluteBackupDirectoryPath) noexcept;

signals:
    void connectionChanged(bool connected);

protected:
    ~LogbookManager() noexcept override;

private:
    const std::unique_ptr<LogbookManagerPrivate> d;

    LogbookManager() noexcept;

    bool connectDb(const QString &logbookPath) noexcept;
    std::pair<bool, Version> checkDatabaseVersion() const noexcept;
};

#endif // LOGBOOKMANAGER_H
