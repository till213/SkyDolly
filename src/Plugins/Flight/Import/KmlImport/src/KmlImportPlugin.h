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
#ifndef KMLIMPORTPLUGIN_H
#define KMLIMPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QWidget>

class QFile;

#include <Flight/FlightAugmentation.h>
#include <PluginManager/FlightImportIntf.h>
#include <PluginManager/FlightImportPluginBase.h>

class Flight;
struct AircraftInfo;
struct FlightCondition;
class FlightImportPluginBaseSettings;
struct KmlImportPluginPrivate;

class KmlImportPlugin : public FlightImportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FLIGHT_IMPORT_INTERFACE_IID FILE "KmlImportPlugin.json")
    Q_INTERFACES(FlightImportIntf)
public:
    KmlImportPlugin() noexcept;
    ~KmlImportPlugin() noexcept override;

protected:
    FlightImportPluginBaseSettings &getPluginSettings() const noexcept override;
    QString getFileExtension() const noexcept override;
    QString getFileFilter() const noexcept override;
    std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    bool importFlight(QFile &file, Flight &flight) noexcept override;

    FlightAugmentation::Procedures getProcedures() const noexcept override;
    FlightAugmentation::Aspects getAspects() const noexcept override;
    QDateTime getStartDateTimeUtc() noexcept override;
    QString getTitle() const noexcept override;
    void updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept override;
    void updateExtendedFlightInfo(Flight &flight) noexcept override;
    void updateExtendedFlightCondition(FlightCondition &flightCondition) noexcept override;

private:
    const std::unique_ptr<KmlImportPluginPrivate> d;

    void parseKML() noexcept;
};

#endif // KMLIMPORTPLUGIN_H
