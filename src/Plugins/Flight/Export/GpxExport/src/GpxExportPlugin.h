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
#ifndef GPXEXPORTPLUGIN_H
#define GPXEXPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QWidget>

class QIODevice;
class QString;
class QDateTime;

#include <Kernel/Settings.h>
#include <PluginManager/FlightExportIntf.h>
#include <PluginManager/FlightExportPluginBase.h>

struct FlightData;
class Aircraft;
struct PositionData;
struct Waypoint;
class FlightExportPluginBaseSettings;
struct GpxExportPluginPrivate;

class GpxExportPlugin : public FlightExportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FLIGHT_EXPORT_INTERFACE_IID FILE "GpxExportPlugin.json")
    Q_INTERFACES(FlightExportIntf)
public:
    GpxExportPlugin() noexcept;
    GpxExportPlugin(const GpxExportPlugin &rhs) = delete;
    GpxExportPlugin(GpxExportPlugin &&rhs) = delete;
    GpxExportPlugin &operator=(const GpxExportPlugin &rhs) = delete;
    GpxExportPlugin &operator=(GpxExportPlugin &&rhs) = delete;
    ~GpxExportPlugin() override;

protected:
    FlightExportPluginBaseSettings &getPluginSettings() const noexcept override;
    QString getFileExtension() const noexcept override;
    QString getFileFilter() const noexcept override;
    std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    bool exportFlightData(const FlightData &flightData, QIODevice &io) const noexcept override;
    bool exportAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept override;

private:
    const std::unique_ptr<GpxExportPluginPrivate> d;

    void updateStartDateTimeUtc(const FlightData &flightData, const Aircraft &aircraft) const noexcept;

    bool exportHeader(QIODevice &io) const noexcept;
    bool exportFlightInfo(const FlightData &flightData, QIODevice &io) const noexcept;
    bool exportAllAircraft(const FlightData &flightData, QIODevice &io) const noexcept;
    bool exportSingleAircraft(const Aircraft &aircraft, QIODevice &io) const noexcept;
    bool exportWaypoints(const FlightData &flightData, QIODevice &io) const noexcept;
    bool exportFooter(QIODevice &io) const noexcept;

    QString getFlightDescription(const FlightData &flightData) const noexcept;
    QString getAircraftDescription(const Aircraft &aircraft) const noexcept;

    inline bool exportTrackPoint(const PositionData &positionData, QIODevice &io) const noexcept;
    inline bool exportWaypoint(const Waypoint &waypoint, QIODevice &io) const noexcept;
};

#endif // GPXEXPORTPLUGIN_H
