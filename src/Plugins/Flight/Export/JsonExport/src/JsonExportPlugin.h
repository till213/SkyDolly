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
#ifndef JSONEXPORTPLUGIN_H
#define JSONEXPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QWidget>

class QIODevice;
class QString;

#include <Kernel/Settings.h>
#include <PluginManager/Flight/FlightExportIntf.h>
#include <PluginManager/Flight/FlightExportPluginBase.h>

struct FlightData;
class Aircraft;
struct PositionData;
struct Waypoint;
class FlightExportPluginBaseSettings;
struct JsonExportPluginPrivate;

class JsonExportPlugin : public FlightExportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FLIGHT_EXPORT_INTERFACE_IID FILE "JsonExportPlugin.json")
    Q_INTERFACES(FlightExportIntf)
public:
    JsonExportPlugin() noexcept;
    JsonExportPlugin(const JsonExportPlugin &rhs) = delete;
    JsonExportPlugin(JsonExportPlugin &&rhs) = delete;
    JsonExportPlugin &operator=(const JsonExportPlugin &rhs) = delete;
    JsonExportPlugin &operator=(JsonExportPlugin &&rhs) = delete;
    ~JsonExportPlugin() override;

protected:
    FlightExportPluginBaseSettings &getPluginSettings() const noexcept override;
    QString getFileExtension() const noexcept override;
    QString getFileFilter() const noexcept override;
    std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    bool exportFlightData(const FlightData &flightData, QIODevice &io) const noexcept override;
    bool exportAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept override;

private:
    const std::unique_ptr<JsonExportPluginPrivate> d;

    bool exportHeader(QIODevice &io) const noexcept;
    bool exportAllAircraft(const FlightData &flightData, QIODevice &io) const noexcept;
    bool exportSingleAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept;
    bool exportWaypoints(const FlightData &flightData, QIODevice &io) const noexcept;
    bool exportFooter(QIODevice &io) const noexcept;

    inline bool exportTrackPoint(const PositionData &positionData, QIODevice &io) const noexcept;
    inline bool exportWaypoint(const auto &waypoint, QIODevice &io) const noexcept;
};

#endif // JSONEXPORTPLUGIN_H
