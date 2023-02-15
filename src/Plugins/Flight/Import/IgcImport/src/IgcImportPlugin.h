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
#ifndef IGCIMPORTPLUGIN_H
#define IGCIMPORTPLUGIN_H

#include <memory>
#include <vector>

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QWidget>

class QIODevice;

#include <Flight/FlightAugmentation.h>
#include <PluginManager/FlightImportIntf.h>
#include <PluginManager/FlightImportPluginBase.h>

struct FlightData;
struct AircraftInfo;
struct FlightCondition;
class FlightImportPluginBaseSettings;
struct IgcImportPluginPrivate;

class IgcImportPlugin : public FlightImportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FLIGHT_IMPORT_INTERFACE_IID FILE "IgcImportPlugin.json")
    Q_INTERFACES(FlightImportIntf)
public:
    IgcImportPlugin() noexcept;
    IgcImportPlugin(const IgcImportPlugin &rhs) = delete;
    IgcImportPlugin(IgcImportPlugin &&rhs) = delete;
    IgcImportPlugin &operator=(const IgcImportPlugin &rhs) = delete;
    IgcImportPlugin &operator=(IgcImportPlugin &&rhs) = delete;
    ~IgcImportPlugin() override;

    std::vector<FlightData> importSelectedFlights(QIODevice &io, bool &ok) noexcept override;

protected:
    FlightImportPluginBaseSettings &getPluginSettings() const noexcept override;
    QString getFileExtension() const noexcept override;
    QString getFileFilter() const noexcept override;
    std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    FlightAugmentation::Procedures getAugmentationProcedures() const noexcept override;
    FlightAugmentation::Aspects getAugmentationAspects() const noexcept override;

private:
    const std::unique_ptr<IgcImportPluginPrivate> d;

    void updateWaypoints(Aircraft &aircraft) noexcept;

    // Estimates the propeller (thrust) lever position, based on the
    // environmentalNoiseLevel and the threshold
    inline double noiseToPosition(double environmentalNoiseLevel, double threshold) noexcept;

    void enrichFlightData(FlightData &flightData) const noexcept;
};

#endif // IGCIMPORTPLUGIN_H
