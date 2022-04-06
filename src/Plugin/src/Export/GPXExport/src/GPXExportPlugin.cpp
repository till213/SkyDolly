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
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../Export.h"
#include "GPXExportOptionWidget.h"
#include "GPXExportSettings.h"
#include "GPXExportPlugin.h"

class GPXExportPluginPrivate
{
public:
    GPXExportPluginPrivate() noexcept
        : flight(Logbook::getInstance().getCurrentFlight())
    {}

    GPXExportSettings settings;
    Flight &flight;
    QDateTime startDateTimeUtc;
    Unit unit;

    static inline const QString FileExtension {QStringLiteral("gpx")};
};

// PUBLIC

GPXExportPlugin::GPXExportPlugin() noexcept
    : d(std::make_unique<GPXExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("GPXExportPlugin::GPXExportPlugin: PLUGIN LOADED");
#endif
}

GPXExportPlugin::~GPXExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("GPXExportPlugin::~GPXExportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

void GPXExportPlugin::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    d->settings.addSettings(keyValues);
}

void GPXExportPlugin::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    d->settings.addKeysWithDefaults(keysWithDefaults);
}

void GPXExportPlugin::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->settings.restoreSettings(valuesByKey);
}

ExportPluginBaseSettings &GPXExportPlugin::getSettings() const noexcept
{
    return d->settings;
}

QString GPXExportPlugin::getFileExtension() const noexcept
{
    return GPXExportPluginPrivate::FileExtension;
}

QString GPXExportPlugin::getFileFilter() const noexcept
{
    return tr("GPS exchange format (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> GPXExportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<GPXExportOptionWidget>(d->settings);
}

bool GPXExportPlugin::writeFile(QIODevice &io) noexcept
{
    io.setTextModeEnabled(true);
    bool ok = exportHeader(io);
    if (ok) {
        ok = exportFlightInfo(io);
    }    
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

void GPXExportPlugin::onRestoreDefaultSettings() noexcept
{
    d->settings.restoreDefaults();
}

// PRIVATE

bool GPXExportPlugin::exportHeader(QIODevice &io) const noexcept
{
    const QString header =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" version=\"1.1\" creator=\"" % Version::getApplicationName() % "\"\n"
"     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
"     xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n";

    return io.write(header.toUtf8());
}

bool GPXExportPlugin::exportFlightInfo(QIODevice &io) const noexcept
{
    const QString header =
"  <metadata>\n"
"    <name><![CDATA[" % d->flight.getTitle() % "]]></name>\n"
"    <desc><![CDATA[" % getFlightDescription() % "]]></desc>\n"
"  </metadata>\n";

    return io.write(header.toUtf8());
}

bool GPXExportPlugin::exportAllAircraft(QIODevice &io) const noexcept
{
    bool ok = true;
    for (const auto &aircraft : d->flight) {
        ok = exportAircraft(*aircraft, io);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool GPXExportPlugin::exportAircraft(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    switch (d->settings.getTimestampMode()) {
    case GPXExportSettings::TimestampMode::Recording:
        d->startDateTimeUtc = d->flight.getAircraftCreationTime(aircraft).toUTC();
        break;
    default:
        d->startDateTimeUtc = d->flight.getAircraftStartZuluTime(aircraft);
        break;
    }

    std::vector<PositionData> interpolatedPositionData;
    resamplePositionDataForExport(aircraft, std::back_inserter(interpolatedPositionData));
    bool ok = true;
    if (interpolatedPositionData.size() > 0) {

        const AircraftInfo &aircraftInfo = aircraft.getAircraftInfoConst();
        const QString trackBegin =
"  <trk>\n"
"    <name><![CDATA[" % aircraftInfo.aircraftType.type % "]]></name>\n"
"    <desc><![CDATA[" % getAircraftDescription(aircraft) % "]]>\n"
"    </desc>\n"
"    <trkseg>\n";

        ok = io.write(trackBegin.toUtf8());
        if (ok) {
            for (PositionData &positionData : interpolatedPositionData) {
                ok = exportTrackPoint(positionData, io);
                if (!ok) {
                    break;
                }
            }
        }
        if (ok) {
            const QString placemarkEnd = QString(
    "    </trkseg>\n"
    "  </trk>\n");
            ok = io.write(placemarkEnd.toUtf8());
        }

    } // size

    return ok;
}

bool GPXExportPlugin::exportWaypoints(QIODevice &io) const noexcept
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

bool GPXExportPlugin::exportFooter(QIODevice &io) const noexcept
{
    const QString footer =
"</gpx>\n";
    return io.write(footer.toUtf8());
}

QString GPXExportPlugin::getFlightDescription() const noexcept
{
    const FlightCondition &flightCondition = d->flight.getFlightConditionConst();
    const QString description =
            d->flight.getDescription() % "\n" %
            "\n" %
            tr("Creation date") % ": " % d->unit.formatDate(d->flight.getCreationTime()) % "\n" %
            tr("Start (local time)") % ": " % d->unit.formatTime(flightCondition.startLocalTime) % "\n" %
            tr("End (local time)") % ": " % d->unit.formatTime(flightCondition.endLocalTime) % "\n" %
            tr("Ambient temperature") % ": " % d->unit.formatCelcius(flightCondition.ambientTemperature) % "\n" %
            tr("Total air temperature") % ": " % d->unit.formatCelcius(flightCondition.totalAirTemperature) % "\n" %
            tr("Precipitation") % ": " % SimType::precipitationStateToString(flightCondition.precipitationState) % "\n" %
            tr("Wind direction") % ": " % d->unit.formatDegrees(flightCondition.windDirection) % "\n" %
            tr("Wind velocity") % ": " % d->unit.formatKnots(flightCondition.windVelocity) % "\n" %
            tr("Visibility") % ": " % d->unit.formatVisibility(flightCondition.visibility) % "\n" %
            tr("In clouds") % ": " % d->unit.formatBoolean(flightCondition.inClouds) % "\n";

    return description;
}

QString GPXExportPlugin::getAircraftDescription(const Aircraft &aircraft) const noexcept
{
    const AircraftInfo &info = aircraft.getAircraftInfoConst();
    const AircraftType &type = info.aircraftType;
    const QString description =
            tr("Category") % ": " % type.category % "\n" %
            tr("Engine type") % ": " % SimType::engineTypeToString(type.engineType) % "\n" %
            tr("Number of engines") % ": " % d->unit.formatNumber(type.numberOfEngines, 0) % "\n" %
            tr("Wingspan") % ": " % d->unit.formatNumber(type.wingSpan, 0) % "\n"
            "\n" %
            tr("Initial altitude above ground") % ": " % d->unit.formatFeet(info.altitudeAboveGround) % "\n" %
            tr("Initial airspeed") % ": " % d->unit.formatKnots(info.initialAirspeed) % "\n" %
            tr("Airline") % ": " % info.airline % "\n" %
            tr("Flight number") % ": " % info.flightNumber % "\n" %
            tr("Tail number") % ": " % info.tailNumber % "\n";
    return description;
}

inline bool GPXExportPlugin::exportTrackPoint(const PositionData &positionData, QIODevice &io) const noexcept
{
    const QDateTime dateTimeUtc = d->startDateTimeUtc.addMSecs(positionData.timestamp);
    const QString trackPoint =
"      <trkpt lat=\"" % Export::formatCoordinate(positionData.latitude) % "\" lon=\"" % Export::formatCoordinate(positionData.longitude) % "\">\n"
"        <ele>" % Export::formatNumber(Convert::feetToMeters(positionData.altitude)).toUtf8() % "</ele>\n"
"        <time>" % dateTimeUtc.toString(Qt::ISODate) % "</time>\n"
"      </trkpt>\n";

    return io.write(trackPoint.toUtf8());
}

inline bool GPXExportPlugin::exportWaypoint(const Waypoint &waypoint, QIODevice &io) const noexcept
{
    const QString waypointString =
"  <wpt lat=\"" % Export::formatCoordinate(waypoint.latitude) % "\" lon=\"" % Export::formatCoordinate(waypoint.longitude) % "\">\n"
"    <ele>" % Export::formatNumber(Convert::feetToMeters(waypoint.altitude)).toUtf8() % "</ele>\n"
"    <time>" % waypoint.zuluTime.toString(Qt::ISODate) % "</time>\n"
"    <name>" % waypoint.identifier % "</name>\n"
"  </wpt>\n";

    return io.write(waypointString.toUtf8());
}
