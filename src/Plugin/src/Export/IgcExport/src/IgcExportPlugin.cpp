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
#include <cstdint>
#include <vector>
#include <iterator>

#include <QtGlobal>
#include <QIODevice>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QString>
#include <QByteArray>
#include <QSysInfo>
#include <QDesktopServices>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/File.h"
#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Kernel/src/Version.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "../../../../src/Export.h"
#include "IgcExportOptionWidget.h"
#include "IgcExportSettings.h"
#include "IgcExportPlugin.h"

namespace
{
    // General
    constexpr char LineEnd[] = "\r\n";
    constexpr char DateFormat[] = "ddMMyy";
    constexpr char TimeFormat[] = "hhmmss";

    // A record
    constexpr char ManufacturerCode[] = "XXY";
    constexpr char SerialId[] = "001";

    // H record
    constexpr char Date[] = "FDTEDATE:";
    constexpr char Pilot[] = "FPLTPILOTINCHARGE:";
    constexpr char CoPilot[] = "FCM2CREW2:";
    constexpr char GliderType[] = "FGTYGLIDERTYPE:";
    constexpr char GliderId[] = "FGIDGLIDERID:";
    constexpr char GPSDatum[] = "FDTMGPSDATUM:WGS84";
    constexpr char FirmwareVersion[] = "FRFWFIRMWAREVERSION:";
    constexpr char HardwareVersion[] = "FRHWHARDWAREVERSION:";
    constexpr char FlightRecorderType[] = "FFTYFRTYPE:";
    constexpr char GpsReceiver[] = "FGPSRECEIVER:";
    constexpr char PressureAltitudeSensor[] = "FPRSPRESSALTSENSOR:";
    constexpr char Security[] = "FFRSSECURITYOK";

    // I record
    constexpr char EnvironmentalNoiseLevel[] = "ENL";

    // C record
    constexpr char ObsoleteFlightDate[] = "000000";
    constexpr char ObsoleteTaskNumber[] = "000000";
    constexpr char TakeoffPoint[] = "TAKEOFF";
    constexpr char StartPoint[] = "START";
    constexpr char TurnPoint[] = "TURN";
    constexpr char FinishPoint[] = "FINISH";
    constexpr char LandingPoint[] = "LANDING";

    // B record
    constexpr char FixValid[] = "A";
}

class IgcExportPluginPrivate
{
public:
    IgcExportPluginPrivate() noexcept
        : flight(nullptr)
    {}

    const Flight *flight;
    IgcExportSettings pluginSettings;
    Unit unit;

    static inline const QByteArray ARecord {"A"};
    static inline const QByteArray HRecord {"H"};
    static inline const QByteArray IRecord {"I"};
    static inline const QByteArray CRecord {"C"};
    static inline const QByteArray BRecord {"B"};
    static inline const QByteArray GRecord {"G"};

    static inline const QString FileSuffix {QStringLiteral("igc")};
};

// PUBLIC

IgcExportPlugin::IgcExportPlugin() noexcept
    : d(std::make_unique<IgcExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("IgcExportPlugin::IgcExportPlugin: PLUGIN LOADED");
#endif
}

IgcExportPlugin::~IgcExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("IgcExportPlugin::~IgcExportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

ExportPluginBaseSettings &IgcExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString IgcExportPlugin::getFileSuffix() const noexcept
{
    return IgcExportPluginPrivate::FileSuffix;
}

QString IgcExportPlugin::getFileFilter() const noexcept
{
    return tr("International gliding commission (*.%1)").arg(getFileSuffix());
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
        ok = exportCRecord(aircraft, io);
    }
    if (ok) {
        ok = exportBRecord(aircraft, io);
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
        IgcExportPluginPrivate::HRecord % ::Date % formatDate(d->flight->getFlightConditionConst().startZuluTime) % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::Pilot % d->pluginSettings.getPilotName().toLatin1() % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::CoPilot % d->pluginSettings.getCoPilotName().toLatin1() % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::GliderType % aircraft.getAircraftInfoConst().aircraftType.type.toLatin1() % ::LineEnd %
        IgcExportPluginPrivate::HRecord % ::GliderId % aircraft.getAircraftInfoConst().tailNumber.toLatin1() % ::LineEnd %
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
    // Write 1 extension: environmental noise levels in bytes 36-38 (of each B record)
    const QByteArray record = IgcExportPluginPrivate::IRecord % "01" % "3638" % ::EnvironmentalNoiseLevel % ::LineEnd;
    return io.write(record);
}

inline bool IgcExportPlugin::exportCRecord(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const FlightPlan &flightPlan = aircraft.getFlightPlanConst();
    const Position &position = aircraft.getPositionConst();
    const int nofTurnPoints = flightPlan.count() - 2;
    QByteArray record = IgcExportPluginPrivate::CRecord % formatDateTime(d->flight->getAircraftStartZuluTime(aircraft)) %
                        ::ObsoleteFlightDate % ::ObsoleteTaskNumber %
                        // Number of turn points, excluding start and end wapoints
                        formatNumber(qMin(nofTurnPoints, 0), 2) %
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

inline bool IgcExportPlugin::exportBRecord(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    QDateTime startTime = d->flight->getAircraftStartZuluTime(aircraft);

    const Engine &engine = aircraft.getEngineConst();
    std::vector<PositionData> interpolatedPositionData;
    resamplePositionDataForExport(aircraft, std::back_inserter(interpolatedPositionData));
    bool ok = true;
    for (PositionData &positionData : interpolatedPositionData) {
        if (!positionData.isNull()) {
            const int altitude = qRound(Convert::feetToMeters(positionData.altitude));
            const QByteArray altitudeByteArray = formatNumber(altitude, 5);
            const int indicatedAltitude = qRound(Convert::feetToMeters(positionData.indicatedAltitude));
            const QByteArray indicatedAltitudeByteArray = formatNumber(indicatedAltitude, 5);
            const EngineData &engineData = engine.interpolate(positionData.timestamp, TimeVariableData::Access::Linear);
            const int noise = estimateEnvironmentalNoise(engineData);
            const QByteArray record = IgcExportPluginPrivate::BRecord %
                                      formatTime(startTime.addMSecs(positionData.timestamp)) %
                                      formatPosition(positionData.latitude, positionData.longitude) %
                                      ::FixValid %
                                      // Pressure altitude
                                      indicatedAltitudeByteArray %
                                      // GNSS altitude
                                      altitudeByteArray %
                                      formatNumber(noise, 3) %
                                      ::LineEnd;
            ok = io.write(record);
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
    int degrees;
    double minutes;
    Convert::dd2dm(latitude, degrees, minutes);
    const int decimals = static_cast<int>((minutes - static_cast<int>(minutes)) * 1000);
    const QByteArray latitudeString = QString("%1%2%3%4")
            .arg(degrees, 2, 10, QLatin1Char('0'))
            .arg(static_cast<int>(minutes), 2, 10, QLatin1Char('0'))
            .arg(decimals, 3, 10, QLatin1Char('0'))
            .arg(latitude >= 0.0 ? QLatin1Char('N') : QLatin1Char('S'))
            .toLatin1();
    return latitudeString;
}

inline QByteArray IgcExportPlugin::IgcExportPlugin::formatLongitude(double longitude) const noexcept
{
    int degrees;
    double minutes;
    Convert::dd2dm(longitude, degrees, minutes);
    const int decimals = static_cast<int>((minutes - static_cast<int>(minutes)) * 1000);
    const QByteArray latitudeString = QString("%1%2%3%4")
            .arg(degrees, 3, 10, QLatin1Char('0'))
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
        noise = qMin(noise, 999);
    } else {
        noise = 0;
    }
    return noise;
}
