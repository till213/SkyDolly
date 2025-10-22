/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#ifndef SDLOGLOCATIONEXPORTPLUGIN_H
#define SDLOGLOCATIONEXPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QWidget>

class QIODevice;
class QString;

#include <Kernel/Settings.h>
#include <PluginManager/Location/LocationExportIntf.h>
#include <PluginManager/Location/LocationExportPluginBase.h>

class Aircraft;
struct LocationData;
struct Waypoint;
class LocationExportPluginBaseSettings;
struct SdLogLocationExportPluginPrivate;

class SdLogLocationExportPlugin : public LocationExportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LOCATION_EXPORT_INTERFACE_IID FILE "SdLogLocationExportPlugin.json")
    Q_INTERFACES(LocationExportIntf)
public:
    SdLogLocationExportPlugin() noexcept;
    SdLogLocationExportPlugin(const SdLogLocationExportPlugin &rhs) = delete;
    SdLogLocationExportPlugin(SdLogLocationExportPlugin &&rhs) = delete;
    SdLogLocationExportPlugin &operator=(const SdLogLocationExportPlugin &rhs) = delete;
    SdLogLocationExportPlugin &operator=(SdLogLocationExportPlugin &&rhs) = delete;
    ~SdLogLocationExportPlugin() override;

protected:
    LocationExportPluginBaseSettings &getPluginSettings() const noexcept override;
    QString getFileExtension() const noexcept override;
    QString getFileFilter() const noexcept override;
    std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    bool exportLocations(const std::vector<Location> &locations, QIODevice &io) const noexcept override;

private:
    const std::unique_ptr<SdLogLocationExportPluginPrivate> d;
};

#endif // SDLOGLOCATIONEXPORTPLUGIN_H
