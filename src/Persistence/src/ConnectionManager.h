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
#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <memory>

#include <QObject>

class QString;

class Metadata;

#include "PersistenceLib.h"

class ConnectionManagerPrivate;

class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    static ConnectionManager &getInstance() noexcept;
    PERSISTENCE_API static void destroyInstance() noexcept;

    bool connectDb(const QString &logbookPath) noexcept;
    void disconnectDb() noexcept;
    bool isConnected() const noexcept;
    const QString &getLogbookPath() const noexcept;

    bool migrate() noexcept;
    bool optimise() noexcept;
    bool backup(const QString &backupLogbookPath) noexcept;
    bool getMetadata(Metadata &metadata) noexcept;

signals:
    void connectionChanged(bool connected);

protected:
    virtual ~ConnectionManager() noexcept;

private:
    Q_DISABLE_COPY(ConnectionManager)
    std::unique_ptr<ConnectionManagerPrivate> d;

    ConnectionManager() noexcept;
};

#endif // CONNECTIONMANAGER_H
