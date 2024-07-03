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
#include <Model/Flight.h>
#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include <Model/FlightCondition.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/SimType.h>
#include <PluginManager/Export.h>
#include "GpxExportOptionWidget.h"
#include "GpxExportSettings.h"
#include "GpxExportPlugin.h"

struct GpxExportPluginPrivate
{
    GpxExportSettings pluginSettings;
    QDateTime startDateTimeUtc;
    Unit unit;
    Convert convert;

    static inline const QString FileExtension {"gpx"};
};

// PUBLIC

GpxExportPlugin::GpxExportPlugin() noexcept
    : d {std::make_unique<GpxExportPluginPrivate>()}
{}

GpxExportPlugin::~GpxExportPlugin() = default;

// PROTECTED

FlightExportPluginBaseSettings &GpxExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString GpxExportPlugin::getFileExtension() const noexcept
{
    return GpxExportPluginPrivate::FileExtension;
}

QString GpxExportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("GPS exchange format (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> GpxExportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<GpxExportOptionWidget>(d->pluginSettings);
}

bool GpxExportPlugin::exportFlightData(const FlightData &flightData, QIODevice &io) const noexcept
{
    io.setTextModeEnabled(true);
    bool ok = exportHeader(io);
    if (ok) {
        ok = exportFlightInfo(flightData, io);
    }
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

bool GpxExportPlugin::exportAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    updateStartDateTimeUtc(flightData, aircraft);
    io.setTextModeEnabled(true);
    bool ok = exportHeader(io);
    if (ok) {
        ok = exportFlightInfo(flightData, io);
    }    
    if (ok) {
        ok = exportWaypoints(flightData, io);
    }
    if (ok) {
        ok = exportSingleAircraft(aircraft, io);
    }    
    if (ok) {
        ok = exportFooter(io);
    }

    return ok;
}

// PRIVATE

void GpxExportPlugin::updateStartDateTimeUtc(const FlightData &flightData, const Aircraft &aircraft) const noexcept
{
    switch (d->pluginSettings.getTimestampMode()) {
    case GpxExportSettings::TimestampMode::Simulation:
        d->startDateTimeUtc = flightData.getAircraftStartZuluTime(aircraft);
        break;
    case GpxExportSettings::TimestampMode::Recording:
        d->startDateTimeUtc = flightData.getAircraftCreationTime(aircraft).toUTC();
        break;
    }
}

bool GpxExportPlugin::exportHeader(QIODevice &io) const noexcept
{
    const QString header =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" version=\"1.1\" creator=\"" % Version::getApplicationName() % "\"\n"
"     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
"     xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n";

    return io.write(header.toUtf8());
}

bool GpxExportPlugin::exportFlightInfo(const FlightData &flightData, QIODevice &io) const noexcept
{
    const QString header =
"  <metadata>\n"
"    <name><![CDATA[" % flightData.title % "]]></name>\n"
"    <desc><![CDATA[" % getFlightDescription(flightData) % "]]></desc>\n"
"  </metadata>\n";

    return io.write(header.toUtf8());
}

bool GpxExportPlugin::exportAllAircraft(const FlightData &flightData, QIODevice &io) const noexcept
{
    bool ok {true};
    for (const auto &aircraft : flightData.aircraft) {
        ok = exportSingleAircraft(aircraft, io);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool GpxExportPlugin::exportSingleAircraft(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const std::vector<PositionData> interpolatedPositionData = Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
    bool ok {true};
    if (interpolatedPositionData.size() > 0) {

        const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();
        const QString trackBegin =
"  <trk>\n"
"    <name><![CDATA[" % aircraftInfo.aircraftType.type % "]]></name>\n"
"    <desc><![CDATA[" % getAircraftDescription(aircraft) % "]]>\n"
"    </desc>\n"
"    <trkseg>\n";

        ok = io.write(trackBegin.toUtf8());
        if (ok) {
            for (const auto &positionData : interpolatedPositionData) {
                ok = exportTrackPoint(positionData, io);
                if (!ok) {
                    break;
                }
            }
        }
        if (ok) {
            const QString placemarkEnd = "    </trkseg>\n"
                                         "  </trk>\n";
            ok = io.write(placemarkEnd.toUtf8());
        }

    } // size

    return ok;
}

bool GpxExportPlugin::exportWaypoints(const FlightData &flightData, QIODevice &io) const noexcept
{
    bool ok {true};
    const FlightPlan &flightPlan = flightData.getUserAircraftConst().getFlightPlan();
    for (const auto &waypoint : flightPlan) {
        ok = exportWaypoint(waypoint, io);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool GpxExportPlugin::exportFooter(QIODevice &io) const noexcept
{
    const QString footer = "</gpx>\n";
    return io.write(footer.toUtf8());
}

QString GpxExportPlugin::getFlightDescription(const FlightData &flightData) const noexcept
{
    const FlightCondition &flightCondition = flightData.flightCondition;
    return flightData.description % "\n" %
           "\n" %
           QObject::tr("Creation date") % ": " % d->unit.formatDate(flightData.creationTime) % "\n" %
           QObject::tr("Flight number") % ": " % flightData.flightNumber % "\n" %
           QObject::tr("Start (local time)") % ": " % d->unit.formatTime(flightCondition.startLocalDateTime) % "\n" %
           QObject::tr("End (local time)") % ": " % d->unit.formatTime(flightCondition.endLocalDateTime) % "\n" %
           QObject::tr("Ambient temperature") % ": " % d->unit.formatCelcius(flightCondition.ambientTemperature) % "\n" %
           QObject::tr("Total air temperature") % ": " % d->unit.formatCelcius(flightCondition.totalAirTemperature) % "\n" %
           QObject::tr("Precipitation") % ": " % SimType::precipitationStateToString(flightCondition.precipitationState) % "\n" %
           QObject::tr("Wind direction") % ": " % d->unit.formatDegrees(flightCondition.windDirection) % "\n" %
           QObject::tr("Wind speed") % ": " % d->unit.formatKnots(flightCondition.windSpeed) % "\n" %
           QObject::tr("Visibility") % ": " % d->unit.formatVisibility(flightCondition.visibility) % "\n" %
           QObject::tr("In clouds") % ": " % d->unit.formatBoolean(flightCondition.inClouds) % "\n";
}

QString GpxExportPlugin::getAircraftDescription(const Aircraft &aircraft) const noexcept
{
    const AircraftInfo &info = aircraft.getAircraftInfo();
    const AircraftType &type = info.aircraftType;
    return QObject::tr("Category") % ": " % type.category % "\n" %
           QObject::tr("Engine type") % ": " % SimType::engineTypeToString(type.engineType) % "\n" %
           QObject::tr("Number of engines") % ": " % d->unit.formatNumber(type.numberOfEngines, 0) % "\n" %
           QObject::tr("Wingspan") % ": " % d->unit.formatFeet(type.wingSpan) % "\n"
           "\n" %
           QObject::tr("Initial altitude above ground") % ": " % d->unit.formatFeet(info.altitudeAboveGround) % "\n" %
           QObject::tr("Initial airspeed") % ": " % d->unit.formatKnots(info.initialAirspeed) % "\n" %
           QObject::tr("Airline") % ": " % info.airline % "\n" %
           QObject::tr("Tail number") % ": " % info.tailNumber % "\n";
}

inline bool GpxExportPlugin::exportTrackPoint(const PositionData &positionData, QIODevice &io) const noexcept
{
    const QDateTime dateTimeUtc = d->startDateTimeUtc.addMSecs(positionData.timestamp);
    // Convert height above EGM geoid to height above WGS84 ellipsoid (HAE) [meters]
    const double heightAboveEllipsoid = d->convert.egmToWgs84Ellipsoid(Convert::feetToMeters(positionData.altitude), positionData.latitude, positionData.longitude);

    const QString trackPoint =
"      <trkpt lat=\"" % Export::formatCoordinate(positionData.latitude) % "\" lon=\"" % Export::formatCoordinate(positionData.longitude) % "\">\n"
"        <ele>" % Export::formatNumber(heightAboveEllipsoid).toUtf8() % "</ele>\n"
"        <time>" % dateTimeUtc.toString(Qt::ISODate) % "</time>\n"
"      </trkpt>\n";

    return io.write(trackPoint.toUtf8());
}

inline bool GpxExportPlugin::exportWaypoint(const Waypoint &waypoint, QIODevice &io) const noexcept
{
    // Convert height above EGM geoid to height above WGS84 ellipsoid (HAE) [meters]
    const double heightAboveEllipsoid = d->convert.egmToWgs84Ellipsoid(Convert::feetToMeters(waypoint.altitude), waypoint.latitude, waypoint.longitude);
    const QString waypointString =
"  <wpt lat=\"" % Export::formatCoordinate(waypoint.latitude) % "\" lon=\"" % Export::formatCoordinate(waypoint.longitude) % "\">\n"
"    <ele>" % Export::formatNumber(heightAboveEllipsoid).toUtf8() % "</ele>\n"
"    <time>" % waypoint.zuluTime.toString(Qt::ISODate) % "</time>\n"
"    <name>" % waypoint.identifier % "</name>\n"
"  </wpt>\n";

    return io.write(waypointString.toUtf8());
}
