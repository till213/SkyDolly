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
#ifndef FLIGHTEXPORTPLUGINBASE_H
#define FLIGHTEXPORTPLUGINBASE_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QStringView>

class QIODevice;

#include <Flight/FlightAugmentation.h>
#include <Kernel/Settings.h>
#include "FlightExportIntf.h"
#include "../DialogPluginBase.h"
#include "../PluginManagerLib.h"

struct PositionData;
class Flight;
struct FlightData;
class Aircraft;
class FlightExportPluginBaseSettings;
struct FlightExportPluginBasePrivate;

class PLUGINMANAGER_API FlightExportPluginBase : public DialogPluginBase, public FlightExportIntf
{
    Q_OBJECT
    Q_INTERFACES(FlightExportIntf)
public:
    FlightExportPluginBase() noexcept;
    FlightExportPluginBase(const FlightExportPluginBase &rhs) = delete;
    FlightExportPluginBase(FlightExportPluginBase &&rhs) = delete;
    FlightExportPluginBase &operator=(const FlightExportPluginBase &rhs) = delete;
    FlightExportPluginBase &operator=(FlightExportPluginBase &&rhs) = delete;
    ~FlightExportPluginBase() override;

    QWidget *getParentWidget() const noexcept final
    {
        return getParentWidget();
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

    bool exportFlight(const Flight &flight)  const noexcept final;

protected:
    // Re-implement
    virtual FlightExportPluginBaseSettings &getPluginSettings() const noexcept = 0;
    virtual QString getFileExtension() const noexcept = 0;
    virtual QString getFileFilter() const noexcept = 0;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept = 0;

    virtual bool exportFlightData(const FlightData &flightData, QIODevice &io) const noexcept = 0;
    virtual bool exportAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept = 0;

private:
    const std::unique_ptr<FlightExportPluginBasePrivate> d;

    bool exportFlight(const Flight &flight, const QString &filePath) const noexcept;
    // Exports all aircraft into separate files, given the 'baseFilePath'
    bool exportAllAircraft(const Flight &flight, const QString &baseFilePath) const noexcept;

    void addSettings(Settings::KeyValues &keyValues) const noexcept final;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept final;
    void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept final;
};

#endif // FLIGHTEXPORTPLUGINBASE_H
