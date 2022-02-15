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
#ifndef SKYCONNECTMANAGER_H
#define SKYCONNECTMANAGER_H

#include <memory>
#include <vector>
#include <utility>
#include <optional>
#include <functional>
#include <cstdint>

#include <QObject>

class QString;
class QUuid;

#include "../../Model/src/TimeVariableData.h"
#include "../../Kernel/src/FlightSimulator.h"
#include "Connect.h"
#include "SkyConnectLib.h"

class SkyConnectIntf;
class skyConnectManagerPrivate;

class SKYCONNECT_API SkyConnectManager : public QObject
{
    Q_OBJECT
public:
    static SkyConnectManager &getInstance() noexcept;
    static void destroyInstance() noexcept;

    /*!
     * The plugin name and the flight simulator it supports.
     */
    typedef struct {
        QString name;
        FlightSimulator::Id flightSimulatorId;
    } SkyConnectPlugin;

    /*!
     * The plugin UUID and the plugin name and capabilities (flight simulator).
     */
    typedef std::pair<QUuid, SkyConnectPlugin> Handle;
    const std::vector<Handle> &initialisePlugins() noexcept;
    const std::vector<Handle> &availablePlugins() const noexcept;
    bool hasPlugins() const noexcept;

    std::optional<std::reference_wrapper<SkyConnectIntf>> getCurrentSkyConnect() const noexcept;
    std::optional<QString> getCurrentSkyConnectPluginName() const noexcept;

public slots:
    bool tryAndSetCurrentSkyConnect(const QUuid &uuid) noexcept;

signals:
    void connectionChanged(SkyConnectIntf *skyConnect);

    void timestampChanged(std::int64_t timestamp, TimeVariableData::Access access);
    void stateChanged(Connect::State state);
    void recordingStopped();

protected:
    virtual ~SkyConnectManager() noexcept;

private:
    Q_DISABLE_COPY(SkyConnectManager)
    std::unique_ptr<skyConnectManagerPrivate> d;

    SkyConnectManager() noexcept;

    void frenchConnection() noexcept;
    void initialisePlugins(const QString &pluginDirectoryName) noexcept;
};

#endif // SKYCONNECTMANAGER_H
