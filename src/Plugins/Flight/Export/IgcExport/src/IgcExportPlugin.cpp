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
#include <cstdint>
#include <cmath>
#include <vector>

#include <QtGlobal>
#include <QIODevice>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QString>
#include <QByteArray>
#include <QSysInfo>
#include <QDesktopServices>

#include <GeographicLib/DMS.hpp>

#include <Kernel/Enum.h>
#include <Kernel/File.h>
#include <Kernel/Unit.h>
#include <Kernel/Convert.h>
#include <Kernel/Version.h>
#include <Kernel/Settings.h>
#include <Kernel/SkyMath.h>
#include <Model/Flight.h>
#include <Model/FlightCondition.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include <PluginManager/Export.h>
#include "IgcExportOptionWidget.h"
#include "IgcExportSettings.h"
#include "IgcExportPlugin.h"

namespace
{
    // General
    constexpr char LineEnd[] {"\r\n"};
    constexpr char DateFormat[] {"ddMMyy"};
    constexpr char TimeFormat[] {"hhmmss"};

    // A record
    constexpr char ManufacturerCode[] {"XXY"};
    constexpr char SerialId[] {"001"};

    // H record
    constexpr char Date[] {"FDTEDATE:"};
    constexpr char Pilot[] {"FPLTPILOTINCHARGE:"};
    constexpr char CoPilot[] {"FCM2CREW2:"};
    constexpr char GliderType[] {"FGTYGLIDERTYPE:"};
    constexpr char GliderId[] {"FGIDGLIDERID:"};
    constexpr char GPSDatum[] {"FDTMGPSDATUM:WGS84"};
    constexpr char FirmwareVersion[] {"FRFWFIRMWAREVERSION:"};
    constexpr char HardwareVersion[] {"FRHWHARDWAREVERSION:"};
    constexpr char FlightRecorderType[] {"FFTYFRTYPE:"};
    constexpr char GpsReceiver[] {"FGPSRECEIVER:"};
    constexpr char PressureAltitudeSensor[] {"FPRSPRESSALTSENSOR:"};
    constexpr char Security[] {"FFRSSECURITYOK"};

    // I record
    constexpr char EnvironmentalNoiseLevel[] {"ENL"};

    // J record
    constexpr char TrueHeading[] {"HDT"};
    constexpr char IndicatedAirspeed[] {"IAS"};

    // C record
    constexpr char ObsoleteFlightDate[] {"000000"};
    constexpr char ObsoleteTaskNumber[] {"000000"};
    constexpr char TakeoffPoint[] {"TAKEOFF"};
    constexpr char StartPoint[] {"START"};
    constexpr char TurnPoint[] {"TURN"};
    constexpr char FinishPoint[] {"FINISH"};
    constexpr char LandingPoint[] {"LANDING"};

    // B record
    constexpr char FixValid[] {"A"};

    // Interval of 20 seconds for K records
    constexpr int KRecordIntervalSec {20};
}

struct IgcExportPluginPrivate
{
    IgcExportPluginPrivate() noexcept
        : flight(nullptr)
    {}

    const Flight *flight;
    IgcExportSettings pluginSettings;
    Unit unit;

    // Listed in typical order of the records
    static inline const QByteArray ARecord {"A"};
    static inline const QByteArray HRecord {"H"};
    static inline const QByteArray IRecord {"I"};
    static inline const QByteArray JRecord {"J"};
    static inline const QByteArray CRecord {"C"};
    static inline const QByteArray BRecord {"B"};
    static inline const QByteArray KRecord {"K"};
    static inline const QByteArray GRecord {"G"};

    static inline const QString FileSuffix {QStringLiteral("igc")};
};

// PUBLIC

IgcExportPlugin::IgcExportPlugin() noexcept
    : d(std::make_unique<IgcExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug() << "IgcExportPlugin::IgcExportPlugin: PLUGIN LOADED";
#endif
}

IgcExportPlugin::~IgcExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug() << "IgcExportPlugin::~IgcExportPlugin: PLUGIN UNLOADED";
#endif
}

// PROTECTED

FlightExportPluginBaseSettings &IgcExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString IgcExportPlugin::getFileSuffix() const noexcept
{
    return IgcExportPluginPrivate::FileSuffix;
}

QString IgcExportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("International gliding commission (*.%1)").arg(getFileSuffix());
}

std::unique_ptr<QWidget> IgcExportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<IgcExportOptionWidget>(d->pluginSettings);
}

bool IgcExportPlugin::hasMultiAircraftSupport() const noexcept
{
    return false;
}

bool IgcExportPlugin::exportFlight([[maybe_unused]] const Flight &flight, [[maybe_unused]] QIODevice &io) noexcept
{
    return false;
}

bool IgcExportPlugin::exportAircraft(const Flight &flight, const Aircraft &aircraft, QIODevice &io) noexcept
{
    d->flight = &flight;
    bool ok = exportARecord(io);
    if (ok) {
        ok = exportHRecord(aircraft, io);
    }
    if (ok) {
        ok = exportIRecord(io);
    }
    if (ok) {
        ok = exportJRecord(io);
    }
    if (ok) {
        ok = exportCRecord(aircraft, io);
    }
    if (ok) {
        ok = exportFixes(aircraft, io);
    }
    if (ok) {
        ok = exportGRecord(io);
    }
    // We are done with the export
    d->flight = nullptr;
    return ok;

}

// PRIVATE

inline bool IgcExportPlugin::exportARecord(QIODevice &io) const noexcept
{
    const QByteArray record = IgcExportPluginPrivate::ARecord % ::ManufacturerCode % ::SerialId % ::LineEnd;
    return io.write(record);
}

inline bool IgcExportPlugin::exportHRecord(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const QByteArray record =
        IgcExportPluginPrivate::HRecord % ::Date % formatDate(d->flight->getFlightCondition().startZuluTime) % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::Pilot % d->pluginSettings.getPilotName().toLatin1() % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::CoPilot % d->pluginSettings.getCoPilotName().toLatin1() % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::GliderType % aircraft.getAircraftInfo().aircraftType.type.toLatin1() % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::GliderId % aircraft.getAircraftInfo().tailNumber.toLatin1() % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::GPSDatum % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::FirmwareVersion % Version::getApplicationVersion().toLatin1() % " with WGS84 Ellipsoid GPS altitude datum" % ::LineEnd %
        // Reporting the kernel version is somewhat arbitrary here - but we have a cool version number value :)
        IgcExportPluginPrivate::HRecord % ::HardwareVersion % QSysInfo::kernelVersion().toLatin1() % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::FlightRecorderType % Version::getApplicationName().toLatin1() % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::GpsReceiver % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::PressureAltitudeSensor % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::Security % ::LineEnd;

    return io.write(record);
}

inline bool IgcExportPlugin::exportIRecord(QIODevice &io) const noexcept
{
    // Write one extension: environmental noise levels in bytes 36-38 (of each B record)
    const QByteArray record = IgcExportPluginPrivate::IRecord % "01" % "3638" % ::EnvironmentalNoiseLevel % ::LineEnd;
    return io.write(record);
}

inline bool IgcExportPlugin::exportJRecord(QIODevice &io) const noexcept
{
    // Write two K value: true heading in bytes 08-10 and indicates airspeed in bytes 11-13 (of each K record)
    const QByteArray record = IgcExportPluginPrivate::JRecord % "0810" % ::TrueHeading % "1113" % ::IndicatedAirspeed % ::LineEnd;
    return io.write(record);
}

inline bool IgcExportPlugin::exportCRecord(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const FlightPlan &flightPlan = aircraft.getFlightPlan();
    const Position &position = aircraft.getPosition();
    const int nofTurnPoints = flightPlan.count() - 2;
    QByteArray record = IgcExportPluginPrivate::CRecord % formatDateTime(d->flight->getAircraftStartZuluTime(aircraft)) %
                        ::ObsoleteFlightDate % ::ObsoleteTaskNumber %
                        // Number of turn points, excluding start and end wapoints
                        formatNumber(std::min(nofTurnPoints, 0), 2) %
                        d->flight->getTitle().toLatin1() % ::LineEnd;
    bool ok = io.write(record);
    const std::size_t count = flightPlan.count();
    std::size_t i = 0;
    while (ok && i < count) {
        const Waypoint &waypoint = flightPlan[i];
        if (i == 0) {
            record = IgcExportPluginPrivate::CRecord % formatPosition(waypoint.latitude, waypoint.longitude);
            record = record % ::TakeoffPoint % " " % waypoint.identifier.toLatin1() % ::LineEnd;
            const PositionData &positionData = position.getFirst();
            record = record % IgcExportPluginPrivate::CRecord % formatPosition(positionData.latitude, positionData.longitude);
            record = record % ::StartPoint % ::LineEnd;
        } else if (i == count - 1) {
            const PositionData &positionData = position.getLast();
            record = IgcExportPluginPrivate::CRecord % formatPosition(positionData.latitude, positionData.longitude);
            record = record % ::FinishPoint % ::LineEnd;
            record = record % IgcExportPluginPrivate::CRecord % formatPosition(waypoint.latitude, waypoint.longitude);
            record = record % ::LandingPoint % " " % waypoint.identifier.toLatin1() % ::LineEnd;
        } else {
            record = IgcExportPluginPrivate::CRecord % formatPosition(waypoint.latitude, waypoint.longitude);
            record = record % ::TurnPoint % " " % waypoint.identifier.toLatin1() % ::LineEnd;
        }
        ++i;
        ok = io.write(record);
    }
    return ok;
}

inline bool IgcExportPlugin::exportFixes(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    QDateTime startTime = d->flight->getAircraftStartZuluTime(aircraft);
    QDateTime lastKFixTime;

    Convert convert;
    Engine &engine = aircraft.getEngine();
    const std::vector<PositionData> interpolatedPositionData = Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
    bool ok {true};
    for (const PositionData &positionData : interpolatedPositionData) {
        if (!positionData.isNull()) {

            // Convert height above EGM geoid to height above WGS84 ellipsoid (HAE) [meters]
            const double heightAboveEllipsoid = convert.egmToWgs84Ellipsoid(Convert::feetToMeters(positionData.altitude), positionData.latitude, positionData.longitude);

            const int gnssAltitude = static_cast<int>(std::round(heightAboveEllipsoid));
            const QByteArray gnssAltitudeByteArray = formatNumber(gnssAltitude, 5);
            const int pressureAltitude = static_cast<int>(std::round(Convert::feetToMeters(positionData.indicatedAltitude)));
            const QByteArray pressureAltitudeByteArray = formatNumber(pressureAltitude, 5);
            const EngineData &engineData = engine.interpolate(positionData.timestamp, TimeVariableData::Access::Linear);
            const int noise = estimateEnvironmentalNoise(engineData);
            const QDateTime currentTime = startTime.addMSecs(positionData.timestamp);
            const QByteArray bRecord = IgcExportPluginPrivate::BRecord %
                                       formatTime(currentTime) %
                                       formatPosition(positionData.latitude, positionData.longitude) %
                                       ::FixValid %
                                       // Pressure altitude
                                       pressureAltitudeByteArray %
                                       // GNSS altitude
                                       gnssAltitudeByteArray %
                                       formatNumber(noise, 3) %
                                       ::LineEnd;
            ok = io.write(bRecord);

            if (ok && (lastKFixTime.isNull() || lastKFixTime.secsTo(currentTime) >= ::KRecordIntervalSec)) {
                const double trueAirspeed = Convert::feetPerSecondToKilometersPerHour(positionData.velocityBodyZ);
                const double indicatedAirspeed = Convert::trueToIndicatedAirspeed(trueAirspeed, positionData.altitude);
                const QByteArray kRecord = IgcExportPluginPrivate::KRecord %
                                           formatTime(currentTime) %
                                           formatNumber(std::round(positionData.trueHeading), 3) %
                                           // IAS: km/h
                                           formatNumber(std::round(indicatedAirspeed), 3) %
                                           ::LineEnd;
                ok = io.write(kRecord);
                lastKFixTime = currentTime;
            }
        }
        if (!ok) {
            break;
        }
    }

    return ok;
}

inline bool IgcExportPlugin::exportGRecord(QIODevice &io) const noexcept
{
    // Empty security record
    const QByteArray record = IgcExportPluginPrivate::GRecord % ::LineEnd;
    return io.write(record);
}

inline QByteArray IgcExportPlugin::formatDate(const QDateTime &date) const noexcept
{
    return date.toString(::DateFormat).toLatin1();
}

inline QByteArray IgcExportPlugin::formatTime(const QDateTime &time) const noexcept
{
    return time.toString(::TimeFormat).toLatin1();
}

inline QByteArray IgcExportPlugin::formatDateTime(const QDateTime &dateTime) const noexcept
{
    return formatDate(dateTime) % formatTime(dateTime);
}

inline QByteArray IgcExportPlugin::IgcExportPlugin::formatNumber(int value, int padding) const noexcept
{
    const QByteArray altitudeString = QStringLiteral("%1").arg(value, padding, 10, QLatin1Char('0')).toLatin1();
    return altitudeString;
}

inline QByteArray IgcExportPlugin::IgcExportPlugin::formatLatitude(double latitude) const noexcept
{
    double degrees;
    double minutes;

    GeographicLib::DMS::Encode(latitude, degrees, minutes);
    const int decimals = static_cast<int>((minutes - static_cast<int>(minutes)) * 1000);
    const QByteArray latitudeString = QString("%1%2%3%4")
            .arg(static_cast<int>(degrees), 2, 10, QLatin1Char('0'))
            .arg(static_cast<int>(minutes), 2, 10, QLatin1Char('0'))
            .arg(decimals, 3, 10, QLatin1Char('0'))
            .arg(latitude >= 0.0 ? QLatin1Char('N') : QLatin1Char('S'))
            .toLatin1();
    return latitudeString;
}

inline QByteArray IgcExportPlugin::IgcExportPlugin::formatLongitude(double longitude) const noexcept
{
    double degrees;
    double minutes;

    GeographicLib::DMS::Encode(longitude, degrees, minutes);
    const int decimals = static_cast<int>((minutes - static_cast<int>(minutes)) * 1000);
    const QByteArray latitudeString = QString("%1%2%3%4")
            .arg(static_cast<int>(degrees), 3, 10, QLatin1Char('0'))
            .arg(static_cast<int>(minutes), 2, 10, QLatin1Char('0'))
            .arg(decimals, 3, 10, QLatin1Char('0'))
            .arg(longitude >= 0.0 ? QLatin1Char('E') : QLatin1Char('W'))
            .toLatin1();
    return latitudeString;
}

inline QByteArray IgcExportPlugin::formatPosition(double latitude, double longitude) const noexcept
{
    return formatLatitude(latitude) % formatLongitude(longitude);
}

inline int IgcExportPlugin::estimateEnvironmentalNoise(const EngineData &engineData) const noexcept
{
    int noise;
    if (engineData.hasCombustion()) {
        noise = static_cast<int>(static_cast<double>(qAbs(engineData.propellerLeverPosition1)) / SkyMath::PositionMax16 * 999.0);
        noise = std::min(noise, 999);
    } else {
        noise = 0;
    }
    return noise;
}
