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
#include <unordered_map>

#include <QObject>

class QIODevice;
class QUuid;
class QWidget;
class QString;

#include <Kernel/QUuidHasher.h>
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

    /*!
     * Imports the flight data with plugin \p pluginUuid from the input device \p io.
     *
     * Implementation note: this method (while not necessarily called from the application
     * code itself) is useful for unit tests. So do not remove.
     *
     * \param pluginUuid
     *        the UUID of the plugin with which to import the flight data
     * \param io
     *        the input device; must already be open for reading
     * \param ok
     *        set to \c true when successful; \c false else
     * \return the list of flight data
     */
    std::vector<FlightData> importFlightData(const QUuid &pluginUuid, QIODevice &io, bool &ok) const noexcept;

    bool exportFlight(const Flight &flight, const QUuid &pluginUuid) const noexcept;
    bool importLocations(const QUuid &pluginUuid) const noexcept;
    bool exportLocations(const QUuid &pluginUuid) const noexcept;

    using PluginRegistry = std::unordered_map<QUuid, QString, QUuidHasher>;

private:
    const std::unique_ptr<PluginManagerPrivate> d;

    PluginManager() noexcept;
    friend std::unique_ptr<PluginManager>::deleter_type;
    ~PluginManager() override;

    //void deleter(SingletonFactory *d) { d->~SingletonFactory(); free(d); }
    std::vector<PluginManager::Handle> enumeratePlugins(const QString &pluginDirectoryName, PluginRegistry &pluginRegistry) noexcept;
};

#endif // PLUGINMANAGER_H
