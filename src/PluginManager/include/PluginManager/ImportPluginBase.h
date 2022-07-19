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
#ifndef IMPORTPLUGINBASE_H
#define IMPORTPLUGINBASE_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QStringView>

class QFile;
class QStringList;

#include <Kernel/Settings.h>
#include <Flight/FlightAugmentation.h>
#include "ImportIntf.h"
#include "PluginBase.h"
#include "PluginManagerLib.h"

class FlightService;
class Flight;
struct AircraftType;
struct AircraftInfo;
struct FlightCondition;
class ImportPluginBaseSettings;
class ImportPluginBasePrivate;

class PLUGINMANAGER_API ImportPluginBase : public PluginBase, public ImportIntf
{
    Q_OBJECT
    Q_INTERFACES(ImportIntf)
public:
    ImportPluginBase() noexcept;
    ~ImportPluginBase() noexcept override;

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

    bool importFlight(FlightService &flightService, Flight &flight) noexcept final;

protected:
    AircraftType &getSelectedAircraftType() const noexcept;

    // Re-implement
    virtual ImportPluginBaseSettings &getPluginSettings() const noexcept = 0;
    virtual QString getFileSuffix() const noexcept = 0;
    virtual QString getFileFilter() const noexcept = 0;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept = 0;
    virtual bool importFlight(QFile &file, Flight &flight) noexcept = 0;

    virtual FlightAugmentation::Procedures getProcedures() const noexcept = 0;
    virtual FlightAugmentation::Aspects getAspects() const noexcept = 0;
    virtual QDateTime getStartDateTimeUtc() noexcept = 0;
    virtual QString getTitle() const noexcept = 0;
    virtual void updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept = 0;
    virtual void updateExtendedFlightInfo(Flight &flight) noexcept = 0;
    virtual void updateExtendedFlightCondition(FlightCondition &flightCondition) noexcept = 0;

private:
    std::unique_ptr<ImportPluginBasePrivate> d;

    virtual void addSettings(Settings::KeyValues &keyValues) const noexcept final;
    virtual void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept final;
    virtual void restoreSettings(Settings::ValuesByKey valuesByKey) noexcept final;

    bool importFlights(const QStringList &filePaths, FlightService &flightService, Flight &flight) noexcept;
    void updateAircraftInfo() noexcept;
    void updateFlightInfo() noexcept;
    void updateFlightCondition() noexcept;
};

#endif // IMPORTPLUGINBASE_H
