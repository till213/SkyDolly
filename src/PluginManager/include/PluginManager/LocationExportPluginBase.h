/**
 * Sky Dolly - The Black Sheep for your Location Recordings
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
#ifndef LOCATIONEXPORTPLUGINBASE_H
#define LOCATIONEXPORTPLUGINBASE_H

#include <memory>
#include <vector>

#include <QObject>
#include <QtPlugin>

#include "LocationExportIntf.h"
#include "PluginBase.h"
#include "PluginManagerLib.h"

class LocationService;
class Location;
struct AircraftType;
struct AircraftInfo;
struct LocationCondition;
class LocationExportPluginBaseSettings;
struct LocationExportPluginBasePrivate;

class PLUGINMANAGER_API LocationExportPluginBase : public PluginBase, public LocationExportIntf
{
    Q_OBJECT
    Q_INTERFACES(LocationExportIntf)
public:
    LocationExportPluginBase() noexcept;
    LocationExportPluginBase(const LocationExportPluginBase &rhs) = delete;
    LocationExportPluginBase(LocationExportPluginBase &&rhs) = delete;
    LocationExportPluginBase &operator=(const LocationExportPluginBase &rhs) = delete;
    LocationExportPluginBase &operator=(LocationExportPluginBase &&rhs) = delete;
    ~LocationExportPluginBase() override;

    QWidget *getParentWidget() const noexcept final
    {
        return PluginBase::getParentWidget();
    }

    void setParentWidget(QWidget *parent) noexcept final
    {
        PluginBase::setParentWidget(parent);
    }

    void storeSettings(const QUuid &pluginUuid) const noexcept final
    {
        PluginBase::storeSettings(pluginUuid);
    }

    void restoreSettings(const QUuid &pluginUuid) noexcept final
    {
        PluginBase::restoreSettings(pluginUuid);
    }

    bool exportLocations(const std::vector<Location> &locations) noexcept final;

protected:
    // Re-implement
    virtual LocationExportPluginBaseSettings &getPluginSettings() const noexcept = 0;
    virtual QString getFileExtension() const noexcept = 0;
    virtual QString getFileFilter() const noexcept = 0;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept = 0;

    virtual bool exportLocations(const std::vector<Location> &locations, QIODevice &io) noexcept = 0;

private:
    const std::unique_ptr<LocationExportPluginBasePrivate> d;

    bool exportLocations(const std::vector<Location> &locations, const QString &filePath) noexcept;

    void addSettings(Settings::KeyValues &keyValues) const noexcept final;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept final;
    void restoreSettings(Settings::ValuesByKey valuesByKey) noexcept final;
};

#endif // LOCATIONEXPORTPLUGINBASE_H
