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

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#include "../../../../../Kernel/src/QStringHasher.h"
#endif

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Kernel/src/File.h"
#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/SampleRate.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../Export.h"
#include "KmlExportOptionWidget.h"
#include "KmlStyleExport.h"
#include "KmlExportPlugin.h"

namespace
{
    // Maximum segments in a <LineString> (resulting in
    // MaxLineSegments + 1 coordinates per <LineString>)
    constexpr int MaxLineSegments = 16384;

    // Placemark "look at" direction
    constexpr char LookAtTilt[] = "50";
    constexpr char LookAtRange[] = "4000";
    constexpr int HeadingNorth = 0;
}

class KmlExportPluginPrivate
{
public:
    KmlExportPluginPrivate() noexcept
        : flight(nullptr),
          styleExport(std::make_unique<KmlStyleExport>(pluginSettings))
    {}

    const Flight *flight;
    std::unique_ptr<KmlStyleExport> styleExport;
    KmlExportSettings pluginSettings;
    Unit unit;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    std::unordered_map<QString, int, QStringHasher> aircraftTypeCount;
#else
    std::unordered_map<QString, int> aircraftTypeCount;
#endif

    static inline const QString FileExtension {QStringLiteral("kml")};
};

// PUBLIC

KmlExportPlugin::KmlExportPlugin() noexcept
    : d(std::make_unique<KmlExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("KmlExportPlugin::KmlExportPlugin: PLUGIN LOADED");
#endif
}

KmlExportPlugin::~KmlExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("KmlExportPlugin::~KmlExportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

ExportPluginBaseSettings &KmlExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString KmlExportPlugin::getFileSuffix() const noexcept
{
    return KmlExportPluginPrivate::FileExtension;
}

QString KmlExportPlugin::getFileFilter() const noexcept
{
    return tr("Keyhole markup language (*.%1)").arg(getFileSuffix());
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
    d->pluginSettings.setNofColorsPerRamp(qMin(nofAircraft, d->pluginSettings.getNofColorsPerRamp()));

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
    const Aircraft &aircraft = d->flight->getUserAircraftConst();
    const PositionData &positionData = aircraft.getPosition().getFirst();
    return exportPlacemark(io, KmlStyleExport::Icon::Airport, d->flight->getTitle(), getFlightDescription(), positionData);
}

bool KmlExportPlugin::exportAllAircraft(QIODevice &io) const noexcept
{
    bool ok = true;
    for (const auto &aircraft : *d->flight) {
        d->aircraftTypeCount[aircraft->getAircraftInfoConst().aircraftType.type] += 1;
        ok = exportAircraft(*aircraft, io);
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

    std::vector<PositionData> interpolatedPositionData;
    resamplePositionDataForExport(aircraft, std::back_inserter(interpolatedPositionData));
    bool ok = true;
    if (interpolatedPositionData.size() > 0) {

        const int aircraftTypeCount = d->aircraftTypeCount[aircraft.getAircraftInfoConst().aircraftType.type];
        const bool isFormation = d->flight->count() > 1;
        const QString aircratId = isFormation ? " #" % d->unit.formatNumber(aircraftTypeCount, 0) : QString();

        const SimType::EngineType engineType = aircraft.getAircraftInfoConst().aircraftType.engineType;
        QString styleMapId = d->styleExport->getNextEngineTypeStyleMap(engineType);
        const QString placemarkBegin = QString(
    "    <Placemark>\n"
    "      <name>" % aircraft.getAircraftInfoConst().aircraftType.type % aircratId % "</name>\n"
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
    bool ok = true;

    const FlightPlan &flightPlan = d->flight->getUserAircraft().getFlightPlanConst();
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
    const FlightCondition &flightCondition = d->flight->getFlightConditionConst();
    const QString description =
            tr("Description") % ": " % d->flight->getDescription() % "\n" %
            "\n" %
            tr("Creation date") % ": " % d->unit.formatDate(d->flight->getCreationTime()) % "\n" %
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

QString KmlExportPlugin::getAircraftDescription(const Aircraft &aircraft) const noexcept
{
    const AircraftInfo &info = aircraft.getAircraftInfoConst();
    const AircraftType &type = info.aircraftType;
    const QString description =
            tr("Category") % ": " % type.category % "\n" %
            tr("Engine type") % ": " % SimType::engineTypeToString(type.engineType) % "\n" %
            tr("Number of engines") % ": " % d->unit.formatNumber(type.numberOfEngines, 0) % "\n" %
            tr("Wingspan") % ": " % d->unit.formatFeet(type.wingSpan) % "\n"
            "\n" %
            tr("Initial altitude above ground") % ": " % d->unit.formatFeet(info.altitudeAboveGround) % "\n" %
            tr("Initial airspeed") % ": " % d->unit.formatKnots(info.initialAirspeed) % "\n" %
            tr("Airline") % ": " % info.airline % "\n" %
            tr("Flight number") % ": " % info.flightNumber % "\n" %
            tr("Tail number") % ": " % info.tailNumber % "\n";
    return description;
}

QString KmlExportPlugin::getWaypointDescription(const Waypoint &waypoint) const noexcept
{
    const QString description =
            tr("Arrival time (local)") % ": " % d->unit.formatTime(waypoint.localTime) % "\n" %
            tr("Arrival time (zulu)") % ": " % d->unit.formatTime(waypoint.zuluTime) % "\n" %
            tr("Altitude") % ": " % d->unit.formatFeet(waypoint.altitude) % "\n";
    return description;
}


inline bool KmlExportPlugin::exportPlacemark(QIODevice &io, KmlStyleExport::Icon icon, const QString &name, const QString &description, const PositionData &positionData) const noexcept
{
    bool ok = !positionData.isNull();
    if (ok) {
        ok = exportPlacemark(io, icon, name, description,
                             positionData.longitude, positionData.latitude, positionData.altitude, positionData.heading);
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

