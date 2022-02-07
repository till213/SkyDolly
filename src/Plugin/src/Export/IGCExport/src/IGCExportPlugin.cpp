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
#include <QCoreApplication>
#include <QFile>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QString>
#include <QByteArray>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDesktopServices>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/File.h"
#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/SimVar.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../../Model/src/AircraftHandle.h"
#include "../../../../../Model/src/AircraftHandleData.h"
#include "../../../../../Model/src/Light.h"
#include "../../../../../Model/src/LightData.h"
#include "../../../../src/Export.h"
#include "IGCExportDialog.h"
#include "IGCExportSettings.h"
#include "IGCExportPlugin.h"

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

    // B record
    constexpr char FixValid[] = "A";
}

class IGCExportPluginPrivate
{
public:
    IGCExportPluginPrivate() noexcept
        : flight(Logbook::getInstance().getCurrentFlight())
    {}

    Flight &flight;
    IGCExportSettings exportSettings;
    Unit unit;

    static const QByteArray ARecord;
    static const QByteArray HRecord;
    static const QByteArray IRecord;
    static const QByteArray CRecord;
    static const QByteArray BRecord;
    static const QByteArray GRecord;
};

const QByteArray IGCExportPluginPrivate::ARecord {"A"};
const QByteArray IGCExportPluginPrivate::HRecord {"H"};
const QByteArray IGCExportPluginPrivate::IRecord {"I"};
const QByteArray IGCExportPluginPrivate::CRecord {"C"};
const QByteArray IGCExportPluginPrivate::BRecord {"B"};
const QByteArray IGCExportPluginPrivate::GRecord {"G"};

// PUBLIC

IGCExportPlugin::IGCExportPlugin() noexcept
    : d(std::make_unique<IGCExportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("IGCExportPlugin::IGCExportPlugin: PLUGIN LOADED");
#endif
}

IGCExportPlugin::~IGCExportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("IGCExportPlugin::~IGCExportPlugin: PLUGIN UNLOADED");
#endif
}

bool IGCExportPlugin::exportData() noexcept
{
    bool ok;
    std::unique_ptr<IGCExportDialog> exportDialog = std::make_unique<IGCExportDialog>(d->exportSettings, getParentWidget());
    const int choice = exportDialog->exec();
    if (choice == QDialog::Accepted) {
        // Remember export path
        const QString exportDirectoryPath = QFileInfo(exportDialog->getSelectedFilePath()).absolutePath();
        Settings::getInstance().setExportPath(exportDirectoryPath);
        const QString filePath = File::ensureSuffix(exportDialog->getSelectedFilePath(), IGCExportDialog::FileSuffix);
        if (!filePath.isEmpty()) {

            QFile file(filePath);
            ok = file.open(QIODevice::WriteOnly);
            if (ok) {
                const Aircraft &aircraft = d->flight.getUserAircraftConst();
                ok = exportIGCFile(aircraft, file);
                file.close();
            }

        } else {
            ok = true;
        }

        if (ok) {
            if (exportDialog->doOpenExportedFile()) {
                const QString fileUrl = QString("file:///") + filePath;
                QDesktopServices::openUrl(QUrl(fileUrl));
            }
        } else {
            QMessageBox::critical(getParentWidget(), tr("Export error"), tr("The IGC file %1 could not be exported.").arg(filePath));
        }

    } else {
        ok = true;
    }

    return ok;
}

// PRIVATE

bool IGCExportPlugin::exportIGCFile(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    bool ok = exportARecord(io);
    if (ok) {
        ok = exportHRecord(aircraft, io);
    }
    if (ok) {
        ok = exportIRecord(io);
    }
    if (ok) {
        ok = exportBRecord(aircraft, io);
    }
    if (ok) {
        ok = exportGRecord(io);
    }
    return ok;
}

inline bool IGCExportPlugin::exportARecord(QIODevice &io) const noexcept
{
    const QByteArray record = IGCExportPluginPrivate::ARecord % ::ManufacturerCode % ::SerialId % ::LineEnd;
    return io.write(record);
}

inline bool IGCExportPlugin::exportHRecord(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    const QByteArray record =
        IGCExportPluginPrivate::HRecord % ::Date % formatDate(d->flight.getFlightConditionConst().startZuluTime) % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::Pilot % "TODO Pilot Name" % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::CoPilot % "TODO CoPilot Name" % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::GliderType % aircraft.getAircraftInfoConst().aircraftType.type.toLatin1() % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::GliderId % aircraft.getAircraftInfoConst().tailNumber.toLatin1() % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::GPSDatum % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::FirmwareVersion % "TODO FirmwareVersion" % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::HardwareVersion % "TODO HardwareVersion" % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::FlightRecorderType % "TODO FlightRecorderType" % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::GpsReceiver % "TODO GpsReceiver" % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::PressureAltitudeSensor % "TODO PressureAltitudeSensor" % ::LineEnd %
        IGCExportPluginPrivate::HRecord % ::Security % ::LineEnd;

    return io.write(record);
}

inline bool IGCExportPlugin::exportIRecord(QIODevice &io) const noexcept
{
    // Write 1 extension: environmental noise levels in bytes 36-38 (of each B record)
    const QByteArray record = IGCExportPluginPrivate::IRecord % "01" % "3638" % ::EnvironmentalNoiseLevel % ::LineEnd;
    return io.write(record);
}

inline bool IGCExportPlugin::exportBRecord(const Aircraft &aircraft, QIODevice &io) const noexcept
{
    bool ok;
    QDateTime startTime = aircraft.getAircraftInfoConst().startDate.toUTC();

    // Position data
    const Position &position = aircraft.getPositionConst();
    ok = true;
    if (d->exportSettings.resamplingPeriod != IGCExportSettings::ResamplingPeriod::Original) {
        const qint64 duration = position.getLast().timestamp;
        qint64 timestamp = 0;
        while (ok && timestamp <= duration) {
            const PositionData &positionData = position.interpolate(timestamp, TimeVariableData::Access::Linear);
            if (!positionData.isNull()) {
                const int altitude = qRound(Convert::feetToMeters(positionData.altitude));
                const int noise = 0;
                const QByteArray record = IGCExportPluginPrivate::BRecord %
                                          formatTime(startTime.addMSecs(timestamp)) %
                                          formatLatitude(positionData.latitude) %
                                          formatLongitude(positionData.longitude) %
                                          ::FixValid %
                                          // TODO Ignore pressure altitude (or properly calculate it somehow)
                                          formatNumber(altitude, 5) %
                                          formatNumber(altitude, 5) %
                                          formatNumber(noise, 3) %
                                          ::LineEnd;
                ok = io.write(record);
            }
            timestamp += Enum::toUnderlyingType(d->exportSettings.resamplingPeriod);
        }
    } else {
        // Original data requested
        for (const PositionData &positionData : position) {
            const int altitude = qRound(Convert::feetToMeters(positionData.altitude));
            const int noise = 0;
            const QByteArray record = IGCExportPluginPrivate::BRecord %
                                      formatTime(startTime.addMSecs(positionData.timestamp)) %
                                      formatLatitude(positionData.latitude) %
                                      formatLongitude(positionData.longitude) %
                                      ::FixValid %
                                      formatNumber(altitude, 5) %
                                      formatNumber(altitude, 5) %
                                      formatNumber(noise, 3) %
                                      ::LineEnd;
            ok = io.write(record);
            if (!ok) {
                break;
            }
        }
    }
    return ok;
}

inline bool IGCExportPlugin::exportGRecord(QIODevice &io) const noexcept
{
    // Empty security record
    const QByteArray record = IGCExportPluginPrivate::GRecord % ::LineEnd;
    return io.write(record);
}

inline QByteArray IGCExportPlugin::formatDate(const QDateTime &date) const noexcept
{
    return date.toString(::DateFormat).toLatin1();
}

inline QByteArray IGCExportPlugin::formatTime(const QDateTime &date) const noexcept
{
    return date.toString(::TimeFormat).toLatin1();
}

inline QByteArray IGCExportPlugin::IGCExportPlugin::formatNumber(int value, int padding) const noexcept
{
    const QByteArray altitudeString = QStringLiteral("%1").arg(value, padding, 10, QLatin1Char('0')).toLatin1();
    return altitudeString;
}

inline QByteArray IGCExportPlugin::IGCExportPlugin::formatLatitude(double latitude) const noexcept
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

inline QByteArray IGCExportPlugin::IGCExportPlugin::formatLongitude(double longitude) const noexcept
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
