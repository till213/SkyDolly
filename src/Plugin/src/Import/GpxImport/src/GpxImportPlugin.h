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
#ifndef GPXIMPORTPLUGIN_H
#define GPXIMPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QWidget>

class QFile;

#include "../../../../../Flight/src/FlightAugmentation.h"
#include "../../../ImportIntf.h"
#include "../../../ImportPluginBase.h"

class Flight;
struct AircraftInfo;
struct FlightCondition;
class ImportPluginBaseSettings;
class GpxImportPluginPrivate;

class GpxImportPlugin : public ImportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IMPORT_INTERFACE_IID FILE "GpxImportPlugin.json")
    Q_INTERFACES(ImportIntf)
public:
    GpxImportPlugin() noexcept;
    virtual ~GpxImportPlugin() noexcept;

protected:
    virtual ImportPluginBaseSettings &getPluginSettings() const noexcept override;
    virtual QString getFileSuffix() const noexcept override;
    virtual QString getFileFilter() const noexcept override;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    virtual bool importFlight(QFile &file, Flight &flight) noexcept override;

    virtual FlightAugmentation::Procedures getProcedures() const noexcept override;
    virtual FlightAugmentation::Aspects getAspects() const noexcept override;
    virtual QDateTime getStartDateTimeUtc() noexcept override;
    virtual QString getTitle() const noexcept override;
    virtual void updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept override;
    virtual void updateExtendedFlightInfo(Flight &flight) noexcept override;
    virtual void updateExtendedFlightCondition(FlightCondition &flightCondition) noexcept override;

private:
    std::unique_ptr<GpxImportPluginPrivate> d;

    void parseGPX() noexcept;
    void updateWaypoints() noexcept;
};

#endif // GPXIMPORTPLUGIN_H
