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
#ifndef KMLEXPORTPLUGIN_H
#define KMLEXPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QUuid>

class QIODevice;
class QString;

#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../ExportIntf.h"
#include "../../../PluginBase.h"
#include "KMLStyleExport.h"

class Flight;
class Aircraft;
struct PositionData;
struct Waypoint;
class KMLExportPluginPrivate;

class KMLExportPlugin : public PluginBase, public ExportIntf
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXPORT_INTERFACE_IID FILE "KMLExportPlugin.json")
    Q_INTERFACES(ExportIntf)
public:
    KMLExportPlugin() noexcept;
    virtual ~KMLExportPlugin() noexcept;

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

    virtual bool exportData() noexcept override;

protected:
    virtual Settings::PluginSettings getSettings() const noexcept override;
    virtual Settings::KeysWithDefaults getKeys() const noexcept override;
    virtual void setSettings(Settings::ValuesByKey) noexcept override;

private:
    std::unique_ptr<KMLExportPluginPrivate> d;

    bool exportHeader(QIODevice &io) const noexcept;
    bool exportFlightInfo(QIODevice &io) const noexcept;
    bool exportAircraft(QIODevice &io) const noexcept;
    bool exportAircraft(const Aircraft &aircraft, QIODevice &io) const noexcept;
    bool exportWaypoints(QIODevice &io) const noexcept;
    bool exportFooter(QIODevice &io) const noexcept;

    QString getFlightDescription() const noexcept;
    QString getAircraftDescription(const Aircraft &aircraft) const noexcept;
    QString getWaypointDescription(const Waypoint &waypoint) const noexcept;

    inline bool exportPlacemark(QIODevice &io, KMLStyleExport::Icon icon, const QString &name, const QString &description,
                               const PositionData &positionData) const noexcept;
    inline bool exportPlacemark(QIODevice &io, KMLStyleExport::Icon icon, const QString &name, const QString &description,
                               double longitude, double latitude, double altitudeInFeet, double heading) const noexcept;

    static inline QString toString(double number) noexcept;
};

#endif // KMLEXPORTPLUGIN_H
