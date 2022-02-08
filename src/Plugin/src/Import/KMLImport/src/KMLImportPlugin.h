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
#ifndef KMLIMPORTPLUGIN_H
#define KMLIMPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QtPlugin>

#include "../../../ImportIntf.h"
#include "../../../PluginBase.h"

class AircraftType;
class KMLImportPluginPrivate;

class KMLImportPlugin : public PluginBase, public ImportIntf
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IMPORT_INTERFACE_IID FILE "KMLImportPlugin.json")
    Q_INTERFACES(ImportIntf)
public:
    KMLImportPlugin() noexcept;
    virtual ~KMLImportPlugin() noexcept;

    virtual QWidget *getParentWidget() const noexcept override
    {
        return PluginBase::getParentWidget();
    }

    virtual void setParentWidget(QWidget *parent) noexcept override
    {
        PluginBase::setParentWidget(parent);
    }

    virtual void storeSettings(const QUuid &pluginUuid) const noexcept override
    {
        PluginBase::storeSettings(pluginUuid);
    }

    virtual void restoreSettings(const QUuid &pluginUuid) noexcept override
    {
        PluginBase::restoreSettings(pluginUuid);
    }

    virtual bool importData(FlightService &flightService) noexcept override;

protected:
    virtual Settings::PluginSettings getSettings() const noexcept override;
    virtual Settings::KeysWithDefaults getKeyWithDefaults() const noexcept override;
    virtual void setSettings(Settings::ValuesByKey) noexcept override;

private:
    std::unique_ptr<KMLImportPluginPrivate> d;

    bool import(const QString &filePath, FlightService &flightService) noexcept;

    void readKML() noexcept;
    void readDocument() noexcept;
    void readPlacemark() noexcept;
    void readWaypoint(const QString &icaoOrName) noexcept;
    void readTrack() noexcept;

    void updateFlightCondition() noexcept;
    void updateAircraftInfo() noexcept;
};

#endif // KMLIMPORTPLUGIN_H
