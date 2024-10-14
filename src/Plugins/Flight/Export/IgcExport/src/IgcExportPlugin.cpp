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
#include <algorithm>
#include <cmath>
#include <cstdlib>

#include <QtGlobal>
#include <QIODevice>
#include <QStringBuilder>
#include <QString>
#include <QStringLiteral>
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
#include <Model/FlightData.h>
#include <Model/FlightCondition.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Attitude.h>
#include <Model/AttitudeData.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include <Model/TimeVariableData.h>
#include <PluginManager/Export.h>
#include "IgcExportOptionWidget.h"
#include "IgcExportSettings.h"
#include "IgcExportPlugin.h"

namespace
{
    // General
    constexpr const char *LineEnd {"\r\n"};
    constexpr const char *DateFormat {"ddMMyy"};
    constexpr const char *TimeFormat {"hhmmss"};

    // A record
    constexpr const char *ManufacturerCode {"XXY"};
    constexpr const char *SerialId {"001"};

    // H record
    constexpr const char *Date {"FDTEDATE:"};
    constexpr const char *Pilot {"FPLTPILOTINCHARGE:"};
    constexpr const char *CoPilot {"FCM2CREW2:"};
    constexpr const char *GliderType {"FGTYGLIDERTYPE:"};
    constexpr const char *GliderId {"FGIDGLIDERID:"};
    constexpr const char *GPSDatum {"FDTMGPSDATUM:WGS84"};
    constexpr const char *FirmwareVersion {"FRFWFIRMWAREVERSION:"};
    constexpr const char *HardwareVersion {"FRHWHARDWAREVERSION:"};
    constexpr const char *FlightRecorderType {"FFTYFRTYPE:"};
    constexpr const char *GpsReceiver {"FGPSRECEIVER:"};
    constexpr const char *PressureAltitudeSensor {"FPRSPRESSALTSENSOR:"};
    constexpr const char *Security {"FFRSSECURITYOK"};

    // I record
    constexpr const char *EnvironmentalNoiseLevel {"ENL"};

    // J record
    constexpr const char *TrueHeading {"HDT"};
    constexpr const char *IndicatedAirspeed {"IAS"};

    // C record
    constexpr const char *ObsoleteFlightDate {"000000"};
    constexpr const char *ObsoleteTaskNumber {"0000"};
    constexpr const char *TakeoffPoint {"TAKEOFF"};
    constexpr const char *StartPoint {"START"};
    constexpr const char *TurnPoint {"TURN"};
    constexpr const char *FinishPoint {"FINISH"};
    constexpr const char *LandingPoint {"LANDING"};

    // B record
    constexpr const char *FixValid {"A"};

    // Interval of 20 seconds for K records
    constexpr int KRecordIntervalSec {20};
}

struct IgcExportPluginPrivate
{
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

    static inline const QString FileExtension {"igc"};
};

// PUBLIC

IgcExportPlugin::IgcExportPlugin() noexcept
    : d {std::make_unique<IgcExportPluginPrivate>()}
{}

IgcExportPlugin::~IgcExportPlugin() = default;

// PROTECTED

FlightExportPluginBaseSettings &IgcExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString IgcExportPlugin::getFileExtension() const noexcept
{
    return IgcExportPluginPrivate::FileExtension;
}

QString IgcExportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("International gliding commission (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> IgcExportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<IgcExportOptionWidget>(d->pluginSettings);
}

bool IgcExportPlugin::exportFlightData([[maybe_unused]] const FlightData &flightData, [[maybe_unused]] QIODevice &io) const noexcept
{
    return false;
}

bool IgcExportPlugin::exportAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    bool ok = exportARecord(io);
    if (ok) {
        ok = exportHRecord(flightData, aircraft, io);
    }
    if (ok) {
        ok = exportIRecord(io);
    }
    if (ok) {
        ok = exportJRecord(io);
    }
    if (ok) {
        ok = exportCRecord(flightData, aircraft, io);
    }
    if (ok) {
        ok = exportFixes(flightData, aircraft, io);
    }
    if (ok) {
        ok = exportGRecord(io);
    }
    return ok;
}

// PRIVATE

inline bool IgcExportPlugin::exportARecord(QIODevice &io) const noexcept
{
    const QByteArray record = IgcExportPluginPrivate::ARecord % ::ManufacturerCode % ::SerialId % ::LineEnd;
    return io.write(record);
}

inline bool IgcExportPlugin::exportHRecord(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const QByteArray record =
        IgcExportPluginPrivate::HRecord % ::Date % formatDate(flightData.flightCondition.getStartZuluDateTime()) % ::LineEnd %
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

inline bool IgcExportPlugin::exportCRecord(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const auto &flightPlan = aircraft.getFlightPlan();
    const Position &position = aircraft.getPosition();
    bool ok {false};
    if (position.count() > 0) {
        const auto waypointCount = flightPlan.count();
        const int nofTurnPoints = std::max(static_cast<int>(waypointCount) - 2, 0);
        // Task declaration header
        QByteArray record = IgcExportPluginPrivate::CRecord % formatDateTime(flightData.getAircraftStartZuluTime(aircraft)) %
                            ::ObsoleteFlightDate % ::ObsoleteTaskNumber %
                            // Number of turn points, excluding start and end wapoints
                            formatNumber(nofTurnPoints, 2) %
                            flightData.title.toLatin1() % ::LineEnd;
        ok = io.write(record);
        std::size_t i {0};
        // Turn points
        while (ok && i < waypointCount) {
            const auto &waypoint = flightPlan[i];
            if (i == 0) {
                const auto &positionData = position.getFirst();
                record = IgcExportPluginPrivate::CRecord % formatPosition(positionData.latitude, positionData.longitude);
                record = record % ::TakeoffPoint % " " % waypoint.identifier.toLatin1() % ::LineEnd;
                record = record % IgcExportPluginPrivate::CRecord % formatPosition(waypoint.latitude, waypoint.longitude);
                record = record % ::StartPoint % ::LineEnd;
            } else if (i == waypointCount - 1) {
                const auto &positionData = position.getLast();
                record = IgcExportPluginPrivate::CRecord % formatPosition(waypoint.latitude, waypoint.longitude);
                record = record % ::FinishPoint % ::LineEnd;
                record = record % IgcExportPluginPrivate::CRecord % formatPosition(positionData.latitude, positionData.longitude);
                record = record % ::LandingPoint % " " % waypoint.identifier.toLatin1() % ::LineEnd;
            } else {
                record = IgcExportPluginPrivate::CRecord % formatPosition(waypoint.latitude, waypoint.longitude);
                record = record % ::TurnPoint % " " % waypoint.identifier.toLatin1() % ::LineEnd;
            }
            ++i;
            ok = io.write(record);
        }
    } // Position count

    return ok;
}

inline bool IgcExportPlugin::exportFixes(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    QDateTime startTime = flightData.getAircraftStartZuluTime(aircraft);
    QDateTime lastKFixTime;

    Convert convert;
    auto &engine = aircraft.getEngine();
    const auto interpolatedPositionData = Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
    bool ok {true};
    for (const auto &positionData : interpolatedPositionData) {
        // Convert height above EGM geoid to height above WGS84 ellipsoid (HAE) [meters]
        const double heightAboveEllipsoid = convert.geoidToEllipsoidHeight(Convert::feetToMeters(positionData.altitude), positionData.latitude, positionData.longitude);

        const auto gnssAltitude = static_cast<int>(std::round(heightAboveEllipsoid));
        const auto gnssAltitudeByteArray = formatNumber(gnssAltitude, 5);
        const auto pressureAltitude = static_cast<int>(std::round(Convert::feetToMeters(positionData.pressureAltitude)));
        const auto pressureAltitudeByteArray = formatNumber(pressureAltitude, 5);
        const auto engineData = engine.interpolate(positionData.timestamp, TimeVariableData::Access::Linear);
        const auto noise = estimateEnvironmentalNoise(engineData);
        const auto currentTime = startTime.addMSecs(positionData.timestamp);
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
            const auto &attitude = aircraft.getAttitude();
            const auto attitudeData = attitude.interpolate(positionData.timestamp, TimeVariableData::Access::NoTimeOffset);
            const auto trueAirspeed = Convert::feetPerSecondToKilometersPerHour(attitudeData.velocityBodyZ);
            const auto indicatedAirspeed = Convert::trueToIndicatedAirspeed(trueAirspeed, positionData.altitude);
            const QByteArray kRecord = IgcExportPluginPrivate::KRecord %
                                       formatTime(currentTime) %
                                       formatNumber(static_cast<int>(std::round(attitudeData.trueHeading)), 3) %
                                       // IAS: km/h
                                       formatNumber(static_cast<int>(std::round(indicatedAirspeed)), 3) %
                                       ::LineEnd;
            ok = io.write(kRecord);
            lastKFixTime = currentTime;
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
    return QStringLiteral("%1").arg(value, padding, 10, QChar('0')).toLatin1();
}

inline QByteArray IgcExportPlugin::IgcExportPlugin::formatLatitude(double latitude) const noexcept
{
    double degrees {0.0};
    double minutes {0.0};

    GeographicLib::DMS::Encode(latitude, degrees, minutes);
    const int decimals = static_cast<int>((minutes - static_cast<int>(minutes)) * 1000);
    const auto value = QStringLiteral("%1%2%3%4")
                           .arg(static_cast<int>(std::abs(degrees)), 2, 10, QChar('0'))
                           .arg(static_cast<int>(std::abs(minutes)), 2, 10, QChar('0'))
                           .arg(std::abs(decimals), 3, 10, QChar('0'))
                           .arg(latitude >= 0.0 ? 'N' : 'S')
                           .toLatin1();
    return value;
}

inline QByteArray IgcExportPlugin::IgcExportPlugin::formatLongitude(double longitude) const noexcept
{
    double degrees {0.0};
    double minutes{0.0};

    GeographicLib::DMS::Encode(longitude, degrees, minutes);
    const int decimals = static_cast<int>((minutes - static_cast<int>(minutes)) * 1000);
    const auto value = QStringLiteral("%1%2%3%4")
                           .arg(static_cast<int>(std::abs(degrees)), 3, 10, QChar('0'))
                           .arg(static_cast<int>(std::abs(minutes)), 2, 10, QChar('0'))
                           .arg(std::abs(decimals), 3, 10, QChar('0'))
                           .arg(longitude >= 0.0 ? 'E' : 'W')
                           .toLatin1();
    return value;
}

inline QByteArray IgcExportPlugin::formatPosition(double latitude, double longitude) const noexcept
{
    return formatLatitude(latitude) % formatLongitude(longitude);
}

inline int IgcExportPlugin::estimateEnvironmentalNoise(const EngineData &engineData) const noexcept
{
    int noise {0};
    if (engineData.hasCombustion()) {
        noise = static_cast<int>(static_cast<double>(std::abs(engineData.propellerLeverPosition1)) / SkyMath::PositionMax16 * 999.0);
        noise = std::min(noise, 999);
    } else {
        noise = 0;
    }
    return noise;
}
