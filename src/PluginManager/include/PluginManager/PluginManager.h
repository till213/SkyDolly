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
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QMap>

class QIODevice;
class QUuid;
class QWidget;
class QString;

#include "PluginManagerLib.h"

class SkyConnectIntf;
class Flight;
struct FlightData;
struct PluginManagerPrivate;

class PLUGINMANAGER_API PluginManager final : public QObject
{
    Q_OBJECT
public:
    PluginManager(const PluginManager &rhs) = delete;
    PluginManager(PluginManager &&rhs) = delete;
    PluginManager &operator=(const PluginManager &rhs) = delete;
    PluginManager &operator=(PluginManager &&rhs) = delete;

    static PluginManager &getInstance() noexcept;
    static void destroyInstance() noexcept;

    void initialise(QWidget *parentWidget) noexcept;

    /*!
     * The plugin UUID and (non-translated) name of the plugin.
     */
    using Handle = std::pair<QUuid, QString>;
    std::vector<Handle> initialiseFlightImportPlugins() noexcept;
    std::vector<Handle> initialiseFlightExportPlugins() noexcept;
    std::vector<Handle> initialiseLocationImportPlugins() noexcept;
    std::vector<Handle> initialiseLocationExportPlugins() noexcept;

    bool importFlights(const QUuid &pluginUuid, Flight &flight) const noexcept;
    std::vector<FlightData> importSelectedFlights(const QUuid &pluginUuid, QIODevice &io, bool &ok) const noexcept;
    bool exportFlight(const Flight &flight, const QUuid &pluginUuid) const noexcept;
    bool importLocations(const QUuid &pluginUuid) const noexcept;
    bool exportLocations(const QUuid &pluginUuid) const noexcept;

private:
    const std::unique_ptr<PluginManagerPrivate> d;

    PluginManager() noexcept;
    ~PluginManager() override;

    std::vector<PluginManager::Handle> enumeratePlugins(const QString &pluginDirectoryName, QMap<QUuid, QString> &plugins) noexcept;
};

#endif // PLUGINMANAGER_H
