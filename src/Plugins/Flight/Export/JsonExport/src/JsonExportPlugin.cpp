/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <memory>
#include <vector>

#include <QIODevice>
#include <QStringBuilder>
#include <QString>
#include <QDateTime>

#include <Kernel/Version.h>
#include <Kernel/Convert.h>
#include <Kernel/File.h>
#include <Kernel/Enum.h>
#include <Kernel/Unit.h>
#include <Kernel/SampleRate.h>
#include <Kernel/Settings.h>
#include <Model/FlightData.h>
#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include <Model/FlightCondition.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/AircraftType.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/SimType.h>
#include <PluginManager/Export.h>
#include "JsonExportSettings.h"
#include "JsonExportPlugin.h"

struct JsonExportPluginPrivate
{
    JsonExportSettings pluginSettings;
    Unit unit;

    static inline const QString FileExtension {"json"};
};

// PUBLIC

JsonExportPlugin::JsonExportPlugin() noexcept
    : d {std::make_unique<JsonExportPluginPrivate>()}
{}

JsonExportPlugin::~JsonExportPlugin() = default;

// PROTECTED

FlightExportPluginBaseSettings &JsonExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString JsonExportPlugin::getFileExtension() const noexcept
{
    return JsonExportPluginPrivate::FileExtension;
}

QString JsonExportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("JavaScript object notation (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> JsonExportPlugin::createOptionWidget() const noexcept
{
    // No custom settings yet
    return nullptr;
}

bool JsonExportPlugin::exportFlightData(const FlightData &flightData, QIODevice &io) const noexcept
{
    io.setTextModeEnabled(true);
    bool ok = exportHeader(io);
    if (ok) {
        ok = exportWaypoints(flightData, io);
    }
    if (ok) {
        ok = exportAllAircraft(flightData, io);
    }
    if (ok) {
        ok = exportFooter(io);
    }
    return ok;
}

bool JsonExportPlugin::exportAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    io.setTextModeEnabled(true);
    bool ok = exportHeader(io);
    if (ok) {
        ok = exportWaypoints(flightData, io);
    }
    if (ok) {
        ok = exportSingleAircraft(flightData, aircraft, io);
        if (ok) {
            ok = io.write("\n");
        }
    }
    if (ok) {
        ok = exportFooter(io);
    }
    return ok;
}

// PRIVATE

bool JsonExportPlugin::exportHeader(QIODevice &io) const noexcept
{
    const QString header =
"{\n"
"  \"type\": \"FeatureCollection\",\n"
"  \"features\": [\n";
    return io.write(header.toUtf8());
}

bool JsonExportPlugin::exportAllAircraft(const FlightData &flightData, QIODevice &io) const noexcept
{
    bool ok {true};
    std::size_t i = 0;
    for (const auto &aircraft : flightData) {
        ok = exportSingleAircraft(flightData, aircraft, io);
        if (ok) {
            if (i < flightData.count() - 1) {
                ok = io.write(",\n");
            } else {
                ok = io.write("\n");
            }
            ++i;
        } else {
            break;
        }
    }
    return ok;
}

bool JsonExportPlugin::exportSingleAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const auto interpolatedPositionData = Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
    bool ok {true};

    const auto &info = aircraft.getAircraftInfo();
    const auto &aircraftType = info.aircraftType;
    const QString trackBegin =
"    {\n"
"      \"type\": \"Feature\",\n"
"      \"geometry\": {\n"
"        \"type\": \"LineString\",\n"
"        \"coordinates\": [\n";
    ok = io.write(trackBegin.toUtf8());
    if (ok) {
        std::size_t i = 0;
        for (const auto &positionData : interpolatedPositionData) {
            ok = exportTrackPoint(positionData, io);
            if (ok) {
                if (i < interpolatedPositionData.size() - 1) {
                    ok = io.write(", ");
                } else {
                    ok = io.write("\n");
                }
                ++i;
            } else {
                break;
            }
        }
    }
    if (ok) {
        const QString placemarkEnd =
"        ]\n"
"      },\n"
"      \"properties\": {\n"
"        \"type\": \"" % aircraftType.type % "\",\n"
"        \"category\": \"" % aircraftType.category % "\",\n"
"        \"engineType\": \"" % SimType::engineTypeToString(aircraftType.engineType) % "\",\n"
"        \"engineCount\": " % QString::number(aircraftType.numberOfEngines) % ",\n"
"        \"wingspanFeet\": " % QString::number(aircraftType.wingSpan) % ",\n"
"        \"initialAltitudeAboveGroundFeet\": " % Export::formatNumber(info.altitudeAboveGround) % ",\n"
"        \"initialAirspeedKnots\": " % QString::number(info.initialAirspeed) % ",\n"
"        \"airline\": \"" % info.airline % "\",\n"
"        \"flightNumber\": \"" % flightData.flightNumber % "\",\n"
"        \"tailNumber\": \"" % info.tailNumber % "\",\n"
"        \"stroke\": \"#ff0000\",\n"
"        \"stroke-width\": \"4\"\n"
"      }\n"
"    }";

      ok = io.write(placemarkEnd.toUtf8());
    }

    return ok;
}

bool JsonExportPlugin::exportWaypoints(const FlightData &flightData, QIODevice &io) const noexcept
{
    bool ok {true};
    const auto &flightPlan = flightData.getUserAircraftConst().getFlightPlan();
    for (const auto &waypoint : flightPlan) {
        ok = exportWaypoint(waypoint, io);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool JsonExportPlugin::exportFooter(QIODevice &io) const noexcept
{
    const QString footer =
"  ]\n"
"}\n";
    return io.write(footer.toUtf8());
}

inline bool JsonExportPlugin::exportTrackPoint(const PositionData &positionData, QIODevice &io) const noexcept
{
    const QString trackPoint = "[" % Export::formatCoordinate(positionData.longitude) % ", " %
                                     Export::formatCoordinate(positionData.latitude) % ", " %
                                     Export::formatNumber(Convert::feetToMeters(positionData.altitude)) %
                               "]";

    return io.write(trackPoint.toUtf8());
}

inline bool JsonExportPlugin::exportWaypoint(const Waypoint &waypoint, QIODevice &io) const noexcept
{
    const QString waypointString =
"    {\n"
"      \"type\": \"Feature\",\n"
"      \"geometry\": {\n"
"        \"type\": \"Point\",\n"
"        \"coordinates\": [" %
           Export::formatCoordinate(waypoint.longitude) % ", " %
           Export::formatCoordinate(waypoint.latitude) % ", " %
           Export::formatNumber(Convert::feetToMeters(waypoint.altitude)) %
           "]\n" %
"      },\n"
"      \"properties\": {\n"
"        \"identifier\": \"" % waypoint.identifier % "\",\n"
"        \"localTime\": \"" % d->unit.formatTime(waypoint.localTime) % "\",\n"
"        \"zuluTime\": \"" % d->unit.formatTime(waypoint.zuluTime) % "\",\n"
"        \"marker-color\": \"#008800\",\n"
"        \"marker-symbol\": \"airport\"\n"
"      }\n"
"    },\n";
    bool ok = io.write(waypointString.toUtf8());

    return ok;
}
