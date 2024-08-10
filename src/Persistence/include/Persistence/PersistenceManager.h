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
#ifndef PERSISTENCEMANAGER_H
#define PERSISTENCEMANAGER_H

#include <memory>

#include <QObject>

class QString;
class QWidget;

class Metadata;
class Version;

#include "PersistenceLib.h"

class Metadata;
class Version;
struct PersistenceManagerPrivate;

class PERSISTENCE_API PersistenceManager final : public QObject
{
    Q_OBJECT
public:
    PersistenceManager(const PersistenceManager &rhs) = delete;
    PersistenceManager(PersistenceManager &&rhs) = delete;
    PersistenceManager &operator=(const PersistenceManager &rhs) = delete;
    PersistenceManager &operator=(PersistenceManager &&rhs) = delete;

    static PersistenceManager &getInstance() noexcept;
    static void destroyInstance() noexcept;

    /*!
     * Connects with the database given by \p logbookPath
     * and initialises the database by applying the required migrations.
     *
     * If a problem with opening the database occurs (e.g. a version
     * mismatch) then a dialog asks the user for alternative logbook
     * paths (or to quit the application altogether).
     *
     * The actual logbook path (which is usually the given \p logbookPath)
     * is stored in the PersistenceManager.
     *
     * \param logbookPath
     *        the path of the logbook (database) file to connect with
     * \return \c true if the connection succeeded; \c false else
     * \sa connectionChanged
     */
    bool connectWithLogbook(const QString &logbookPath, QWidget *parent) noexcept;

    /*!
     * Disconnects with the application logbook and also removes the default database
     * connection.
     *
     * \sa Const#DefaultConnectionName
     */
    void disconnectFromLogbook() noexcept;
    bool isConnected() const noexcept;
    QString getLogbookPath() const noexcept;

    bool optimise() const noexcept;

    Metadata getMetadata(bool *ok = nullptr) const noexcept;
    Version getDatabaseVersion(bool *ok = nullptr) const noexcept;
    QString getBackupDirectoryPath(bool *ok = nullptr) const noexcept;

signals:
    /*!
     * Emitted whenver the connection to the logbook (database) has changed.
     *
     * \param connected
     *        \c true if a successful connection has been made; \c false else (disconnected)
     */
    void connectionChanged(bool connected);

    /*!
     * Emitted whenever locations have been imported into the logbook.
     */
    void locationsImported();

private:
    const std::unique_ptr<PersistenceManagerPrivate> d;

    PersistenceManager() noexcept;
    friend std::unique_ptr<PersistenceManager>::deleter_type;
    ~PersistenceManager() override;
};

#endif // PERSISTENCEMANAGER_H
