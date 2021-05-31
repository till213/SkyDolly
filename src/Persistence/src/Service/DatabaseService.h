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
#ifndef DATABASESERVICE_H
#define DATABASESERVICE_H

#include <QObject>

class QString;
class QWidget;

#include "../Metadata.h"
#include "../PersistenceLib.h"

class Version;
class DatabaseServicePrivate;

class PERSISTENCE_API DatabaseService : public QObject
{
    Q_OBJECT
public:
    DatabaseService(QObject *parent = nullptr) noexcept;
    virtual ~DatabaseService() noexcept;

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
     * \sa logbookConnectionChanged
     */
    bool connectWithLogbook(const QString &logbookPath, QWidget *parent) noexcept;
    void disconnectFromLogbook() noexcept;
    bool isConnected() const noexcept;
    const QString &getLogbookPath() const noexcept;

    bool optimise() noexcept;
    bool backup() noexcept;
    bool getMetadata(Metadata &metadata) const noexcept;

    static QString getExistingLogbookPath(QWidget *parent) noexcept;
    static QString getNewLogbookPath(QWidget *parent) noexcept;

private:
    bool checkDatabaseVersion(Version &databaseVdersion) const noexcept;

signals:
    void logbookConnectionChanged(bool connected);
};

#endif // DATABASESERVICE_H
