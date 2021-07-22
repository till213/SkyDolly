/**
 * Sky Dolly - The black sheep for your flight recordings
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

#include <QCoreApplication>
#include <QIODevice>
#include <QFile>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "KMLExportPlugin.h"

namespace
{
    // Resampling at 1 Hz
    constexpr int ResamplePeriodMSec = 1000;
    // Precision of exported double values
    constexpr int NumberPrecision = 12;
    constexpr char LookAtTilt[] = "50";
    constexpr char LookAtRange[] = "4000";
}

class KMLExportPluginPrivate
{
public:
    KMLExportPluginPrivate() noexcept
        : flight(Logbook::getInstance().getCurrentFlight())
    {}

    Flight &flight;
    Unit unit;
};

// PUBLIC

KMLExportPlugin::KMLExportPlugin() noexcept
    : d(std::make_unique<KMLExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("KMLExportPlugin::KMLExportPlugin: PLUGIN LOADED");
#endif
}

KMLExportPlugin::~KMLExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("KMLExportPlugin::~KMLExportPlugin: PLUGIN UNLOADED");
#endif
}

bool KMLExportPlugin::exportData() const noexcept
{
    bool ok;
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    QString exportPath = Settings::getInstance().getExportPath();

    const QString filePath = QFileDialog::getSaveFileName(getParentWidget(), QCoreApplication::translate("KMLExportPlugin", "Export KML"), exportPath, QString("*.kml"));
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        ok = file.open(QIODevice::WriteOnly);
        if (ok) {
            file.setTextModeEnabled(true);
            ok = exportHeader(file);
        }
        if (ok) {
            ok = exportStyles(file);
        }
        if (ok) {
            ok = exportFlightInfo(file);
        }
        if (ok) {
            ok = exportAircrafts(file);
        }
        if (ok) {
            ok = exportFooter(file);
        }
        file.close();

        if (ok) {
            exportPath = QFileInfo(filePath).absolutePath();
            Settings::getInstance().setExportPath(exportPath);
        } else {
            QMessageBox::critical(getParentWidget(), QCoreApplication::translate("KMLExportPlugin", "Export error"), QString(QCoreApplication::translate("KMLExportPlugin", "The KML file %1 could not be written.")).arg(filePath));
        }
    } else {
        ok = true;
    }
    return ok;
}

// PRIVATE

bool KMLExportPlugin::exportHeader(QIODevice &io) const noexcept
{
    const QString header =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n"
"  <Document>\n"
"    <name>" % d->flight.getTitle() % "</name>\n";

    return io.write(header.toUtf8());
}

bool KMLExportPlugin::exportStyles(QIODevice &io) const noexcept
{
    const QString header =

// Polyline style

"    <Style id=\"s_flight_h\">\n"
"      <LineStyle>\n"
"        <color>ff00ffff</color>\n"
"        <width>1.5</width>\n"
"      </LineStyle>\n"
"      <PolyStyle>\n"
"        <color>ff7faadd</color>\n"
"          <colorMode>normal</colorMode>\n"
"          <outline>0</outline>\n"
"        </PolyStyle>\n"
"      </Style>\n"
"      <Style id=\"s_flight\">\n"
"        <LineStyle>\n"
"          <color>ff0000ff</color>\n"
"          <width>1.5</width>\n"
"        </LineStyle>\n"
"        <PolyStyle>\n"
"          <color>7f7faaaa</color>\n"
"          <colorMode>normal</colorMode>\n"
"          <outline>0</outline>\n"
"        </PolyStyle>\n"
"      </Style>\n"
"      <StyleMap id=\"sm_flight\">\n"
"        <Pair>\n"
"          <key>normal</key>\n"
"          <styleUrl>#s_flight</styleUrl>\n"
"        </Pair>\n"
"        <Pair>\n"
"          <key>highlight</key>\n"
"          <styleUrl>#s_flight_h</styleUrl>\n"
"        </Pair>\n"
"      </StyleMap>\n"

// Placemark style

"      <Style id=\"s_airports\">\n"
"        <IconStyle>\n"
"          <scale>1.2</scale>\n"
"          <Icon><href>http://maps.google.com/mapfiles/kml/shapes/airports.png</href></Icon>\n"
"          <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
"        </IconStyle>\n"
"	   </Style>\n"
"      <Style id=\"s_airports_h\">\n"
"        <IconStyle>\n"
"          <scale>1.4</scale>\n"
"          <Icon><href>http://maps.google.com/mapfiles/kml/shapes/airports.png</href></Icon>\n"
"          <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
"        </IconStyle>\n"
"	   </Style>\n"
"      <StyleMap id=\"sm_airports\">\n"
"        <Pair>\n"
"          <key>normal</key>\n"
"          <styleUrl>#s_airports</styleUrl>\n"
"        </Pair>\n"
"        <Pair>\n"
"          <key>highlight</key>\n"
"          <styleUrl>#s_airports_h</styleUrl>\n"
"        </Pair>\n"
"      </StyleMap>\n";

    return io.write(header.toUtf8());
}

bool KMLExportPlugin::exportFlightInfo(QIODevice &io) const noexcept
{
    const Aircraft &aircraft = d->flight.getUserAircraftConst();
    const PositionData &positionData = aircraft.getPosition().getFirst();
    bool ok = !positionData.isNull();
    if (ok) {
        const QString placemark =
"      <Placemark>\n"
"        <name>" % d->flight.getTitle() % "</name>\n"
"        <description>" % getFlightDescription() % "</description>\n"
"        <LookAt>\n"
"          <longitude>" % toString(positionData.longitude) % "</longitude>\n"
"          <latitude>" % toString(positionData.latitude) % "</latitude>\n"
"          <altitude>" % toString(Convert::feetToMeters(positionData.altitude)) % "</altitude>\n"
"          <heading>" % toString(positionData.heading) % "</heading>\n"
"          <tilt>" % LookAtTilt % "</tilt>\n"
"          <range>" % LookAtRange % "</range>\n"
"          <altitudeMode>absolute</altitudeMode>\n"
"        </LookAt>\n"
"        <styleUrl>#sm_airports</styleUrl>\n"
"        <Point>\n"
"          <extrude>1</extrude>\n"
"          <altitudeMode>absolute</altitudeMode>\n"
"          <gx:drawOrder>1</gx:drawOrder>\n"
"          <coordinates>" % toString(positionData.longitude) % "," % toString(positionData.latitude) % "," % toString(Convert::feetToMeters(positionData.altitude)) % "</coordinates>\n"
"        </Point>\n"
"      </Placemark>\n";
        ok = io.write(placemark.toUtf8());
    }
    return ok;
}

bool KMLExportPlugin::exportAircrafts(QIODevice &io) const noexcept
{
    bool ok = true;
    for (const auto &aircraft : d->flight) {
        ok = exportAircraf(*aircraft, io);
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool KMLExportPlugin::exportAircraf(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const PositionData positionData;
    const QString placemarkBegin = QString(
"    <Placemark>\n"
"      <name>" % aircraft.getAircraftInfoConst().aircraftType.type % "</name>\n"
"      <description>" % getAircraftDescription(aircraft) % "</description>\n"
"      <styleUrl>#sm_flight</styleUrl>\n"
"      <LineString>\n"
"        <extrude>1</extrude>\n"
"        <tessellate>1</tessellate>\n"
"        <altitudeMode>absolute</altitudeMode>\n"
"        <coordinates>\n");
    bool ok = io.write(placemarkBegin.toUtf8());
    if (ok) {
        // Position data
        const Position &position = aircraft.getPositionConst();
        const qint64 duration = position.getLast().timestamp;

        qint64 time = 0;
        while (time <= duration && ok) {
            const PositionData &positionData = position.interpolate(time, TimeVariableData::Access::Linear);
            if (!positionData.isNull()) {
                ok = io.write((toString(positionData.longitude) % "," %
                               toString(positionData.latitude) % "," %
                               toString(Convert::feetToMeters(positionData.altitude))).toUtf8() % " ");
            }
            time += ResamplePeriodMSec;
        }
    }
    if (ok) {
        const QString placemarkEnd = QString(
"\n"
"        </coordinates>\n"
"      </LineString>\n"
"    </Placemark>\n");
        ok = io.write(placemarkEnd.toUtf8());
    }
    return ok;
}

bool KMLExportPlugin::exportFooter(QIODevice &io) const noexcept
{
    const QString footer =
"  </Document>\n"
"</kml>\n";
    return io.write(footer.toUtf8());
}

QString KMLExportPlugin::getFlightDescription() const noexcept
{
    const FlightCondition &flightCondition = d->flight.getFlightConditionConst();
    const QString description =
            tr("Description") % ": " % d->flight.getDescription() % "\n" %
            "\n" %
            tr("Creation date") % ": " % d->unit.formatDate(d->flight.getCreationDate()) % "\n" %
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

QString KMLExportPlugin::getAircraftDescription(const Aircraft &aircraft) const noexcept
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

inline QString KMLExportPlugin::toString(double number) noexcept
{
    return QString::number(number, 'g', NumberPrecision);
}
