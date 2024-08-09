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
#ifndef LOCATIONIMPORTPLUGINBASE_H
#define LOCATIONIMPORTPLUGINBASE_H

#include <memory>
#include <vector>

#include <QObject>
#include <QtPlugin>

class QWidget;
class QIODevice;

#include "LocationImportIntf.h"
#include "../DialogPluginBase.h"
#include "../PluginManagerLib.h"

struct Location;
struct AircraftType;
struct AircraftInfo;
struct LocationCondition;
class LocationImportPluginBaseSettings;
struct LocationImportPluginBasePrivate;

class PLUGINMANAGER_API LocationImportPluginBase : public QObject, public LocationImportIntf, public DialogPluginBase
{
    Q_OBJECT
    Q_INTERFACES(LocationImportIntf)
public:
    LocationImportPluginBase() noexcept;
    LocationImportPluginBase(const LocationImportPluginBase &rhs) = delete;
    LocationImportPluginBase(LocationImportPluginBase &&rhs) = delete;
    LocationImportPluginBase &operator=(const LocationImportPluginBase &rhs) = delete;
    LocationImportPluginBase &operator=(LocationImportPluginBase &&rhs) = delete;
    ~LocationImportPluginBase() override;

    QWidget *getParentWidget() const noexcept final
    {
        return DialogPluginBase::getParentWidget();
    }

    void setParentWidget(QWidget *parent) noexcept final
    {
        DialogPluginBase::setParentWidget(parent);
    }

    void storeSettings(const QUuid &pluginUuid) const noexcept final
    {
        DialogPluginBase::storeSettings(pluginUuid);
    }

    void restoreSettings(const QUuid &pluginUuid) noexcept final
    {
        DialogPluginBase::restoreSettings(pluginUuid);
    }

    bool importLocations() noexcept final;

protected:
    // Re-implement
    virtual LocationImportPluginBaseSettings &getPluginSettings() const noexcept = 0;
    virtual QString getFileExtension() const noexcept = 0;
    virtual QString getFileFilter() const noexcept = 0;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept = 0;
    virtual std::vector<Location> importLocations(QIODevice &io, bool &ok) noexcept = 0;

    void addSettings(Settings::KeyValues &keyValues) const noexcept final;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept final;
    void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept final;

private:
    const std::unique_ptr<LocationImportPluginBasePrivate> d;

    bool importLocations(const QStringList &filePaths) noexcept;
    bool storeLocations(std::vector<Location> &locations) const noexcept;    
};

#endif // LOCATIONIMPORTPLUGINBASE_H
