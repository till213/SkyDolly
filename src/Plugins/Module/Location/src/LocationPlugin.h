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
#ifndef LOCATIONPLUGIN_H
#define LOCATIONPLUGIN_H

#include <QObject>
#include <QString>

class QWidget;

#include <Model/Location.h>
#include "PluginManager/AbstractModule.h"

struct InitialPosition;
class ModuleBaseSettings;
struct LocationPluginPrivate;

class LocationPlugin : public AbstractModule
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MODULE_INTERFACE_IID FILE "LocationPlugin.json")
    Q_INTERFACES(ModuleIntf)
public:
    explicit LocationPlugin(QObject *parent = nullptr) noexcept;
    LocationPlugin(const LocationPlugin &rhs) = delete;
    LocationPlugin(LocationPlugin &&rhs) = delete;
    LocationPlugin &operator=(const LocationPlugin &rhs) = delete;
    LocationPlugin &operator=(LocationPlugin &&rhs) = delete;
    ~LocationPlugin() override;

    QString getModuleName() const noexcept override;
    QWidget *getWidget() const noexcept override;

protected:
    ModuleBaseSettings &getPluginSettings() const noexcept override;

private:
    const std::unique_ptr<LocationPluginPrivate> d;

    void frenchConnection() noexcept;

private slots:
    void captureLocation() noexcept;
    void updateLocation() noexcept;
    void teleportTo(const Location &location) noexcept;
    void onLocationReceived(Location location) noexcept;
};

#endif // LOCATIONPLUGIN_H
