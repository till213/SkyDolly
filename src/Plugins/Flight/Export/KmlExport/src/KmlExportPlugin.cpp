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
#include <algorithm>
#include <memory>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include <QIODevice>
#include <QStringBuilder>
#include <QString>

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
#include "KmlExportOptionWidget.h"
#include "KmlStyleExport.h"
#include "KmlExportPlugin.h"

namespace
{
    // Maximum segments in a <LineString> (resulting in
    // MaxLineSegments + 1 coordinates per <LineString>)
    constexpr int MaxLineSegments = 16384;

    // Placemark "look at" direction
    constexpr const char *LookAtTilt {"50"};
    constexpr const char *LookAtRange {"4000"};
    constexpr int HeadingNorth = 0;
}

struct KmlExportPluginPrivate
{
    const Flight *flight {nullptr};
    KmlExportSettings pluginSettings;
    std::unique_ptr<KmlStyleExport> styleExport {std::make_unique<KmlStyleExport>(pluginSettings)};
    Unit unit;
    std::unordered_map<QString, int> aircraftTypeCount;

    static constexpr const char *FileExtension {"kml"};
};

// PUBLIC

KmlExportPlugin::KmlExportPlugin() noexcept
    : d(std::make_unique<KmlExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug() << "KmlExportPlugin::KmlExportPlugin: PLUGIN LOADED";
#endif
}

KmlExportPlugin::~KmlExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug() << "KmlExportPlugin::~KmlExportPlugin: PLUGIN UNLOADED";
#endif
}

// PROTECTED

FlightExportPluginBaseSettings &KmlExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString KmlExportPlugin::getFileExtension() const noexcept
{
    return KmlExportPluginPrivate::FileExtension;
}

QString KmlExportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Keyhole markup language (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> KmlExportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<KmlExportOptionWidget>(d->pluginSettings);
}

bool KmlExportPlugin::hasMultiAircraftSupport() const noexcept
{
    // We can store multiple LineStrings in the KML format
    return true;
}

bool KmlExportPlugin::exportFlight(const Flight &flight, QIODevice &io) noexcept
{
    io.setTextModeEnabled(true);

    d->flight = &flight;
    d->aircraftTypeCount.clear();
    const int nofAircraft = d->flight->count();
    // Only create as many colors per ramp as there are aircraft (if there are less aircraft
    // than requested colors per ramp)
    d->pluginSettings.setNofColorsPerRamp(std::min(nofAircraft, d->pluginSettings.getNofColorsPerRamp()));

    bool ok = exportHeader(io);
    if (ok) {
        ok = d->styleExport->exportStyles(io);
    }
    if (ok) {
        ok = exportFlightInfo(io);
    }
    if (ok) {
        ok = exportAllAircraft(io);
    }
    if (ok) {
        ok = exportWaypoints(io);
    }
    if (ok) {
        ok = exportFooter(io);
    }
    // We are done with the export
    d->flight = nullptr;

    return ok;
}

bool KmlExportPlugin::exportAircraft(const Flight &flight, const Aircraft &aircraft, QIODevice &io) noexcept
{
    io.setTextModeEnabled(true);

    d->flight = &flight;
    d->aircraftTypeCount.clear();
    // Only create as many colors per ramp as there are aircraft (if there are less aircraft
    // than requested colors per ramp)
    d->pluginSettings.setNofColorsPerRamp(1);

    bool ok = exportHeader(io);
    if (ok) {
        ok = d->styleExport->exportStyles(io);
    }
    if (ok) {
        ok = exportFlightInfo(io);
    }
    if (ok) {
        ok = exportAircraft(aircraft, io);
    }
    if (ok) {
        ok = exportWaypoints(io);
    }
    if (ok) {
        ok = exportFooter(io);
    }
    // We are done with the export
    d->flight = nullptr;

    return ok;
}

// PRIVATE

bool KmlExportPlugin::exportHeader(QIODevice &io) const noexcept
{
    const QString header =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n"
"  <Document>\n"
"    <name><![CDATA[" % d->flight->getTitle() % "]]></name>\n";

    return io.write(header.toUtf8());
}

bool KmlExportPlugin::exportFlightInfo(QIODevice &io) const noexcept
{
    const Aircraft &aircraft = d->flight->getUserAircraft();
    const PositionData &positionData = aircraft.getPosition().getFirst();
    return exportPlacemark(io, KmlStyleExport::Icon::Airport, d->flight->getTitle(), getFlightDescription(), positionData);
}

bool KmlExportPlugin::exportAllAircraft(QIODevice &io) const noexcept
{
    bool ok {true};
    for (const auto &aircraft : *d->flight) {
        d->aircraftTypeCount[aircraft.getAircraftInfo().aircraftType.type] += 1;
        ok = exportAircraft(aircraft, io);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool KmlExportPlugin::exportAircraft(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const QString lineStringBegin = QString(
"        <LineString>\n"
"          <extrude>1</extrude>\n"
"          <tessellate>1</tessellate>\n"
"          <altitudeMode>absolute</altitudeMode>\n"
"          <coordinates>\n");
    const QString lineStringEnd = QString(
"\n"
"          </coordinates>\n"
"        </LineString>\n");

    std::vector<PositionData> interpolatedPositionData = Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
    bool ok {true};
    if (interpolatedPositionData.size() > 0) {

        const int aircraftTypeCount = d->aircraftTypeCount[aircraft.getAircraftInfo().aircraftType.type];
        const bool isFormation = d->flight->count() > 1;
        const QString aircratId = isFormation ? " #" % d->unit.formatNumber(aircraftTypeCount, 0) : QString();

        const SimType::EngineType engineType = aircraft.getAircraftInfo().aircraftType.engineType;
        QString styleMapId = d->styleExport->getNextEngineTypeStyleMap(engineType);
        const QString placemarkBegin = QString(
    "    <Placemark>\n"
    "      <name>" % aircraft.getAircraftInfo().aircraftType.type % aircratId % "</name>\n"
    "      <description>" % getAircraftDescription(aircraft) % "</description>\n"
    "      <styleUrl>#" % styleMapId % "</styleUrl>\n"
    "      <MultiGeometry>\n"
    );

        ok = io.write(placemarkBegin.toUtf8());
        if (ok) {

            const int interpolatedPositionCount = interpolatedPositionData.size();
            int nextLineSegmentIndex = 0;
            int currentIndex = nextLineSegmentIndex;
            while (currentIndex < interpolatedPositionCount - 1) {
                if (currentIndex == nextLineSegmentIndex) {
                    // End the previous line segment (if any)
                    if (currentIndex > 0) {
                        ok = io.write(lineStringEnd.toUtf8());
                    }
                    // Start a new line segment
                    if (ok) {
                        ok = io.write(lineStringBegin.toUtf8());
                    }
                    if (!ok) {
                        break;
                    }
                    // Update the index of the next line segment start, but
                    // don't increment the currentIndex just yet: the
                    // last point of the previous line segment is repeated,
                    // in order to connect the segments
                    nextLineSegmentIndex += ::MaxLineSegments;
                } else {
                    currentIndex += 1;
                }
                const PositionData positionData = interpolatedPositionData[currentIndex];
                ok = io.write((Export::formatCoordinate(positionData.longitude) % "," %
                               Export::formatCoordinate(positionData.latitude) % "," %
                               Export::formatCoordinate(Convert::feetToMeters(positionData.altitude))).toUtf8() % " ");
                if (!ok) {
                    break;
                }
            }
            if (ok) {
                ok = io.write(lineStringEnd.toUtf8());
            }

        }
        if (ok) {
            const QString placemarkEnd = QString(
    "      </MultiGeometry>\n"
    "    </Placemark>\n");
            ok = io.write(placemarkEnd.toUtf8());
        }

    } // size

    return ok;
}

bool KmlExportPlugin::exportWaypoints(QIODevice &io) const noexcept
{
    bool ok {true};

    const FlightPlan &flightPlan = d->flight->getUserAircraft().getFlightPlan();
    for (const Waypoint &waypoint : flightPlan) {
        ok = exportPlacemark(io, KmlStyleExport::Icon::Flag, waypoint.identifier, getWaypointDescription(waypoint),
                             waypoint.longitude, waypoint.latitude, waypoint.altitude, HeadingNorth);
    }
    return ok;
}

bool KmlExportPlugin::exportFooter(QIODevice &io) const noexcept
{
    const QString footer =
"  </Document>\n"
"</kml>\n";
    return io.write(footer.toUtf8());
}

QString KmlExportPlugin::getFlightDescription() const noexcept
{
    const FlightCondition &flightCondition = d->flight->getFlightCondition();
    const QString description =
            QObject::tr("Description") % ": " % d->flight->getDescription() % "\n" %
            "\n" %
            QObject::tr("Creation date") % ": " % d->unit.formatDate(d->flight->getCreationTime()) % "\n" %
            QObject::tr("Start (local time)") % ": " % d->unit.formatTime(flightCondition.startLocalTime) % "\n" %
            QObject::tr("End (local time)") % ": " % d->unit.formatTime(flightCondition.endLocalTime) % "\n" %
            QObject::tr("Ambient temperature") % ": " % d->unit.formatCelcius(flightCondition.ambientTemperature) % "\n" %
            QObject::tr("Total air temperature") % ": " % d->unit.formatCelcius(flightCondition.totalAirTemperature) % "\n" %
            QObject::tr("Precipitation") % ": " % SimType::precipitationStateToString(flightCondition.precipitationState) % "\n" %
            QObject::tr("Wind direction") % ": " % d->unit.formatDegrees(flightCondition.windDirection) % "\n" %
            QObject::tr("Wind speed") % ": " % d->unit.formatKnots(flightCondition.windSpeed) % "\n" %
            QObject::tr("Visibility") % ": " % d->unit.formatVisibility(flightCondition.visibility) % "\n" %
            QObject::tr("In clouds") % ": " % d->unit.formatBoolean(flightCondition.inClouds) % "\n";

    return description;
}

QString KmlExportPlugin::getAircraftDescription(const Aircraft &aircraft) const noexcept
{
    const AircraftInfo &info = aircraft.getAircraftInfo();
    const AircraftType &type = info.aircraftType;
    const QString description =
            QObject::tr("Category") % ": " % type.category % "\n" %
            QObject::tr("Engine type") % ": " % SimType::engineTypeToString(type.engineType) % "\n" %
            QObject::tr("Number of engines") % ": " % d->unit.formatNumber(type.numberOfEngines, 0) % "\n" %
            QObject::tr("Wingspan") % ": " % d->unit.formatFeet(type.wingSpan) % "\n"
            "\n" %
            QObject::tr("Initial altitude above ground") % ": " % d->unit.formatFeet(info.altitudeAboveGround) % "\n" %
            QObject::tr("Initial airspeed") % ": " % d->unit.formatKnots(info.initialAirspeed) % "\n" %
            QObject::tr("Airline") % ": " % info.airline % "\n" %
            QObject::tr("Flight number") % ": " % info.flightNumber % "\n" %
            QObject::tr("Tail number") % ": " % info.tailNumber % "\n";
    return description;
}

QString KmlExportPlugin::getWaypointDescription(const Waypoint &waypoint) const noexcept
{
    const QString description =
            QObject::tr("Arrival time (local)") % ": " % d->unit.formatTime(waypoint.localTime) % "\n" %
            QObject::tr("Arrival time (zulu)") % ": " % d->unit.formatTime(waypoint.zuluTime) % "\n" %
            QObject::tr("Altitude") % ": " % d->unit.formatFeet(waypoint.altitude) % "\n";
    return description;
}


inline bool KmlExportPlugin::exportPlacemark(QIODevice &io, KmlStyleExport::Icon icon, const QString &name, const QString &description, const PositionData &positionData) const noexcept
{
    bool ok = !positionData.isNull();
    if (ok) {
        ok = exportPlacemark(io, icon, name, description,
                             positionData.longitude, positionData.latitude, positionData.altitude, positionData.trueHeading);
    }
    return ok;
}

inline bool KmlExportPlugin::exportPlacemark(QIODevice &io, KmlStyleExport::Icon icon, const QString &name, const QString &description,
                                             double longitude, double latitude, double altitudeInFeet, double heading) const noexcept
{
    const QString placemark =
"    <Placemark>\n"
"      <name><![CDATA[" % name % "]]></name>\n"
"      <description><![CDATA[" % description % "]]></description>\n"
"      <LookAt>\n"
"        <longitude>" % Export::formatCoordinate(longitude) % "</longitude>\n"
"        <latitude>" % Export::formatCoordinate(latitude) % "</latitude>\n"
"        <altitude>" % Export::formatCoordinate(Convert::feetToMeters(altitudeInFeet)) % "</altitude>\n"
"        <heading>" % Export::formatCoordinate(heading) % "</heading>\n"
"        <tilt>" % LookAtTilt % "</tilt>\n"
"        <range>" % LookAtRange % "</range>\n"
"        <altitudeMode>absolute</altitudeMode>\n"
"      </LookAt>\n"
"      <styleUrl>" % d->styleExport->getStyleUrl(icon) %"</styleUrl>\n"
"      <Point>\n"
"        <extrude>1</extrude>\n"
"        <altitudeMode>absolute</altitudeMode>\n"
"        <gx:drawOrder>1</gx:drawOrder>\n"
"        <coordinates>" % Export::formatCoordinate(longitude) % "," % Export::formatCoordinate(latitude) % "," % Export::formatCoordinate(Convert::feetToMeters(altitudeInFeet)) % "</coordinates>\n"
"      </Point>\n"
"    </Placemark>\n";
    return io.write(placemark.toUtf8());
}

