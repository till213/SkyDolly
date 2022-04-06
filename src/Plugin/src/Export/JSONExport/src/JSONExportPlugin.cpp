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
#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <cstdint>

#include <QIODevice>
#include <QStringBuilder>
#include <QString>
#include <QDateTime>

#include "../../../../../Kernel/src/Version.h"
#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Kernel/src/File.h"
#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/SampleRate.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/AircraftInfo.h"
#include "../../../../../Model/src/AircraftType.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../Export.h"
#include "JSONExportSettings.h"
#include "JSONExportPlugin.h"

class JSONExportPluginPrivate
{
public:
    JSONExportPluginPrivate() noexcept
        : flight(Logbook::getInstance().getCurrentFlight())
    {}

    JSONExportSettings settings;
    Flight &flight;
    Unit unit;

    static inline const QString FileExtension {QStringLiteral("json")};
};

// PUBLIC

JSONExportPlugin::JSONExportPlugin() noexcept
    : d(std::make_unique<JSONExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("JSONExportPlugin::JSONExportPlugin: PLUGIN LOADED");
#endif
}

JSONExportPlugin::~JSONExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("JSONExportPlugin::~JSONExportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

ExportPluginBaseSettings &JSONExportPlugin::getSettings() const noexcept
{
    return d->settings;
}

QString JSONExportPlugin::getFileExtension() const noexcept
{
    return JSONExportPluginPrivate::FileExtension;
}

QString JSONExportPlugin::getFileFilter() const noexcept
{
    return tr("JavaScript object notation (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> JSONExportPlugin::createOptionWidget() const noexcept
{
    // No custom settings yet
    return nullptr;
}

bool JSONExportPlugin::writeFile(QIODevice &io) noexcept
{
    io.setTextModeEnabled(true);
    bool ok = exportHeader(io);
    if (ok) {
        ok = exportWaypoints(io);
    }
    if (ok) {
        ok = exportAllAircraft(io);
    }
    if (ok) {
        ok = exportFooter(io);
    }

    return ok;
}

// PROTECTED SLOTS

void JSONExportPlugin::onRestoreDefaultSettings() noexcept
{
    d->settings.restoreDefaults();
}

// PRIVATE

bool JSONExportPlugin::exportHeader(QIODevice &io) const noexcept
{
    const QString header =
"{\n"
"  \"type\": \"FeatureCollection\",\n"
"  \"features\": [\n";
    return io.write(header.toUtf8());
}

bool JSONExportPlugin::exportAllAircraft(QIODevice &io) const noexcept
{
    bool ok = true;
    std::size_t i = 0;
    for (const auto &aircraft : d->flight) {
        ok = exportAircraft(*aircraft, io);
        if (ok) {
            if (i < d->flight.count() - 1) {
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

bool JSONExportPlugin::exportAircraft(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    std::vector<PositionData> interpolatedPositionData;
    resamplePositionDataForExport(aircraft, std::back_inserter(interpolatedPositionData));
    bool ok = true;

    const AircraftInfo &info = aircraft.getAircraftInfoConst();
    const AircraftType &type = info.aircraftType;
    const QString trackBegin = QString(
"    {\n"
"      \"type\": \"Feature\",\n"
"      \"geometry\": {\n"
"        \"type\": \"LineString\",\n"
"        \"coordinates\": [\n");
    ok = io.write(trackBegin.toUtf8());
    if (ok) {
        std::size_t i = 0;
        for (PositionData &positionData : interpolatedPositionData) {
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
        const QString placemarkEnd = QString(
"        ]\n"
"      },\n"
"      \"properties\": {\n"
"        \"type\": \"" % type.type % "\",\n"
"        \"category\": \"" % type.category % "\",\n"
"        \"engineType\": \"" % SimType::engineTypeToString(type.engineType) % "\",\n"
"        \"engineCount\": " % QString::number(type.numberOfEngines) % ",\n"
"        \"wingspanFeet\": " % QString::number(type.wingSpan) % ",\n"
"        \"initialAltitudeAboveGroundFeet\": " % QString::number(info.altitudeAboveGround) % ",\n"
"        \"initialAirspeedKnots\": " % QString::number(info.initialAirspeed) % ",\n"
"        \"airline\": \"" % info.airline % "\",\n"
"        \"flightNumber\": \"" % info.flightNumber % "\",\n"
"        \"tailNumber\": \"" % info.tailNumber % "\",\n"
"        \"stroke\": \"#ff0000\",\n"
"        \"stroke-width\": \"4\"\n"
"      }\n"
"    }");

      ok = io.write(placemarkEnd.toUtf8());
    }

    return ok;
}

bool JSONExportPlugin::exportWaypoints(QIODevice &io) const noexcept
{
    bool ok = true;
    const FlightPlan &flightPlan = d->flight.getUserAircraft().getFlightPlanConst();
    for (const Waypoint &waypoint : flightPlan) {
        ok = exportWaypoint(waypoint, io);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool JSONExportPlugin::exportFooter(QIODevice &io) const noexcept
{
    const QString footer =
"  ]\n"
"}\n";
    return io.write(footer.toUtf8());
}

inline bool JSONExportPlugin::exportTrackPoint(const PositionData &positionData, QIODevice &io) const noexcept
{
    const QString trackPoint = "[" % Export::formatCoordinate(positionData.longitude) % ", " %
                                     Export::formatCoordinate(positionData.latitude) % ", " %
                                     Export::formatNumber(Convert::feetToMeters(positionData.altitude)) %
                               "]";

    return io.write(trackPoint.toUtf8());
}

inline bool JSONExportPlugin::exportWaypoint(const Waypoint &waypoint, QIODevice &io) const noexcept
{
    const QString waypointString = QString(
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
"    },\n");
    bool ok = io.write(waypointString.toUtf8());

    return ok;
}
