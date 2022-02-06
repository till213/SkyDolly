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
#ifndef CSVEXPORTPLUGIN_H
#define CSVEXPORTPLUGIN_H

#include <QObject>
#include <QtPlugin>

class QString;

#include "../../../ExportIntf.h"
#include "../../../PluginBase.h"

class Aircraft;
class PositionData;
class EngineData;
class PrimaryFlightControlData;
class SecondaryFlightControlData;
class AircraftHandleData;
class LightData;
class CSVExportPrivate;

class CSVExportPrivate;

class CSVExportPlugin : public PluginBase, public ExportIntf
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXPORT_INTERFACE_IID FILE "CSVExportPlugin.json")
    Q_INTERFACES(ExportIntf)
public:
    static const QString FileSuffix;

    CSVExportPlugin() noexcept;
    virtual ~CSVExportPlugin() noexcept;

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

private:
    CSVExportPrivate *d;

    static QString getPositionHeader() noexcept;
    static QString getPositionData(const PositionData &data) noexcept;

    static QString getEngineHeader() noexcept;
    static QString getEngineData(const EngineData &data) noexcept;

    static QString getPrimaryFlightControlHeader() noexcept;
    static QString getPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept;

    static QString getSecondaryFlightControlHeader() noexcept;
    static QString getSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept;

    static QString getAircraftHandleHeader() noexcept;
    static QString getAircraftHandleData(const AircraftHandleData &data) noexcept;

    static QString getLighteHeader() noexcept;
    static QString getLightData(const LightData &data) noexcept;
};

#endif // CSVEXPORTPLUGIN_H
