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
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <memory>
#include <utility>
#include <vector>

#include <QObject>
#include <QMap>

class QUuid;
class QWidget;
class QString;

#include "ExportIntf.h"
#include "PluginLib.h"

class SkyConnectIntf;
class FlightService;
class PluginManagerPrivate;

class PLUGIN_API PluginManager : public QObject
{
    Q_OBJECT
public:
    static PluginManager &getInstance() noexcept;
    static void destroyInstance() noexcept;

    void initialise(QWidget *parentWidget);

    /*!
     * The plugin UUID and (non-translated) name of the plugin.
     */
    typedef std::pair<QUuid, QString> Handle;
    std::vector<Handle> initialiseExportPlugins() noexcept;
    std::vector<Handle> initialiseImportPlugins() noexcept;

    bool importData(const QUuid &pluginUuid, FlightService &flightService) const noexcept;
    bool exportData(const QUuid &pluginUuid) const noexcept;

protected:
    virtual ~PluginManager() noexcept;

private:
    Q_DISABLE_COPY(PluginManager)
    std::unique_ptr<PluginManagerPrivate> d;

    PluginManager() noexcept;

    std::vector<PluginManager::Handle> enumeratePlugins(const QString &pluginDirectoryName, QMap<QUuid, QString> &plugins) noexcept;
};

#endif // PLUGINMANAGER_H
