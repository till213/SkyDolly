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
#ifndef EXPORTPLUGINBASE_H
#define EXPORTPLUGINBASE_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QStringView>

class QFile;

#include "../../Kernel/src/Settings.h"
#include "../../Flight/src/FlightAugmentation.h"
#include "ExportIntf.h"
#include "PluginBase.h"
#include "PluginLib.h"

class FlightService;
class Flight;
struct AircraftType;
struct AircraftInfo;
struct FlightCondition;
class ExportPluginBasePrivate;

class PLUGIN_API ExportPluginBase : public PluginBase, public ExportIntf
{
    Q_OBJECT
    Q_INTERFACES(ExportIntf)
public:
    ExportPluginBase() noexcept;
    virtual ~ExportPluginBase() noexcept;

    virtual QWidget *getParentWidget() const noexcept override final
    {
        return PluginBase::getParentWidget();
    }

    virtual void setParentWidget(QWidget *parent) noexcept override final
    {
        PluginBase::setParentWidget(parent);
    }

    virtual void storeSettings(const QUuid &pluginUuid) const noexcept override final
    {
        PluginBase::storeSettings(pluginUuid);
    }

    virtual void restoreSettings(const QUuid &pluginUuid) noexcept override final
    {
        PluginBase::restoreSettings(pluginUuid);
    }

    virtual bool exportData() noexcept override final;

protected:
    AircraftType &getSelectedAircraftType() const noexcept;

    // Re-implement
    virtual QString getFileFilter() const noexcept = 0;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept = 0;

    virtual bool readFile(QFile &file) noexcept = 0;

    virtual FlightAugmentation::Procedures getProcedures() const noexcept = 0;
    virtual FlightAugmentation::Aspects getAspects() const noexcept = 0;
    virtual QDateTime getStartDateTimeUtc() noexcept = 0;
    virtual QString getTitle() const noexcept = 0;
    virtual void updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept = 0;
    virtual void updateExtendedFlightInfo(Flight &flight) noexcept = 0;
    virtual void updateExtendedFlightCondition(FlightCondition &flightCondition) noexcept = 0;

protected slots:
    virtual void onRestoreDefaultSettings() noexcept = 0;

private:
    std::unique_ptr<ExportPluginBasePrivate> d;

    bool ExportFile(const QString &filePath, FlightService &flightService) noexcept;
    void updateAircraftInfo() noexcept;
    void updateFlightInfo() noexcept;
    void updateFlightCondition() noexcept;
};

#endif // EXPORTPLUGINBASE_H
