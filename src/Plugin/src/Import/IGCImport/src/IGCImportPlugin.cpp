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
#include <tuple>
#include <vector>

#include <QStringBuilder>
#include <QIODevice>
#include <QFlags>
#include <QByteArray>
#include <QList>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <QDateTime>
#include <QTimeZone>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringView>

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Model/src/SimVar.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
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
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "../../../../../Flight/src/FlightAugmentation.h"
#include "../../../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../../../SkyConnect/src/SkyConnectIntf.h"
#include "IGCImportPlugin.h"

namespace
{
    // Timestamp (msec), latitude (degrees), longitude (degrees), altitude (feet)
    typedef std::tuple<qint64, double, double, double> TrackItem;

    // One hour threshold
    constexpr int DayChangeThresholdSeconds = 60 * 60;

    // Record types
    constexpr char ARecord = 'A';
    constexpr char HRecord = 'H';
    constexpr char CRecord = 'C';
    constexpr char BRecord = 'B';

    // Three letter codes (TLC)
    constexpr char TLCDate[] = "DTE";
    constexpr char TLCPilot[] = "PLT";
    constexpr char TLCCoPilot[] = "CM2";
    constexpr char TLCGliderType[] = "GTY";
    constexpr char TLCGliderId[] = "GID";

    // Formats
    constexpr char DateFormat[] = "HHmmss";

    // H (header) record
    constexpr char HRecordDatePattern[] = "^HFDTE(?:DATE:)?(\\d{2})(\\d{2})(\\d{2})(?:,?(\\d{2}))?";
    constexpr char HRecordPilotPattern[] = "^H(\\w)PLT(?:.{0,}?:(.*)|(.*))$";
    constexpr char HRecordCoPilotPattern[] = "^H(\\w)CM2(?:.{0,}?:(.*)|(.*))$";
    constexpr char HRecordGliderTypePattern[] = "^H(\\w)GTY(?:.{0,}?:(.*)|(.*))$";
    constexpr char HRecordGliderIdPattern[] = "^H(\\w)GID(?:.{0,}?:(.*)|(.*))$";

    constexpr int HRecordDayIndex = 1;
    constexpr int HRecordMonthIndex = 2;
    constexpr int HRecordYearIndex = 3;
    constexpr int HRecordFlightNumberIndex = 4;

    // C (task) record
    constexpr char CRecordTaskDefinitionPattern[] = "^C(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{4})([-\\d]{2})(.*)";
    constexpr char CRecordTaskPattern[] = "^C(\\d{2})(\\d{5})([NS])(\\d{3})(\\d{5})([EW])(.*)";

    constexpr int CRecordLatitudeDegreesIndex = 1;
    // MMmmm - minutes (MM) with fractional (mmm) part: by dividing by 1000 we get the proper float value
    constexpr int CRecordLatitudeMinutesIndex = 2;
    // N(orth) or S(outh)
    constexpr int CRecordLatitudeDirectionIndex = 3;

    constexpr int CRecordLongitudeDegreesIndex = 4;
    // MMmmm - minutes (MM) with fractional (mmm) part: by dividing by 1000 we get the proper float value
    constexpr int CRecordLongitudeMinutesIndex = 5;
    // E(ast) or W(est)
    constexpr int CRecordLongitudeDirectionIndex = 6;
    // Task text
    constexpr int CRecordTaskIndex = 7;

    // B (fix) record
    constexpr char BRecordPattern[] = "^B(\\d{6})(\\d{2})(\\d{5})([NS])(\\d{3})(\\d{5})([EW])([AV])(-\\d{4}|\\d{5})(-\\d{4}|\\d{5})";
    // HHMMSS
    constexpr int BRecordDateIndex = 1;

    constexpr int BRecordLatitudeDegreesIndex = 2;
    // MMmmm - minutes (MM) with fractional (mmm) part: by dividing by 1000 we get the proper float value
    constexpr int BRecordLatitudeMinutesIndex = 3;
    // N(orth) or S(outh)
    constexpr int BRecordLatitudeDirectionIndex = 4;

    constexpr int BRecordLongitudeDegreesIndex = 5;
    // MMmmm - minutes (MM) with fractional (mmm) part: by dividing by 1000 we get the proper float value
    constexpr int BRecordLongitudeMinutesIndex = 6;
    // E(ast) or W(est)
    constexpr int BRecordLongitudeDirectionIndex = 7;

    // Pressure altitude (in metres, relative to the ICAO ISA 1013.25 HPa datum)
    [[maybe_unused]]
    constexpr int BRecordPressureAltitudeIndex = 8;
    // GNSS altitude (in metres, above the WGS84 ellipsoid)
    constexpr int BRecordGNSSAltitudeIndex = 9;

    // Values
    [[maybe_unused]]
    constexpr char DirectionTypeNorth = 'N';
    constexpr char DirectionTypeSouth = 'S';
    [[maybe_unused]]
    constexpr char DirectionTypeEast = 'E';
    constexpr char DirectionTypeWest = 'W';
}

class IGCImportPluginPrivate
{
public:
    IGCImportPluginPrivate()
        : file(nullptr),
          currentWaypointTimestamp(0),
          hRecordDateRegExp(QString(::HRecordDatePattern)),
          hRecordPilotRegExp(QString(::HRecordPilotPattern)),
          hRecordCoPilotRegExp(QString(::HRecordCoPilotPattern)),
          hRecordGliderTypeRegExp(QString(::HRecordGliderTypePattern)),
          hRecordGliderIdRegExp(QString(::HRecordGliderIdPattern)),
          cRecordTaskDefinitionRegExp(QString(::CRecordTaskDefinitionPattern)),
          cRecordTaskRegExp(QString(::CRecordTaskPattern)),
          bRecordRegExp(QString(::BRecordPattern))
    {}

    QFile *file;
    Unit unit;
    qint64 currentWaypointTimestamp;

    // Header data
    QDate flightDate;
    QString flightNumber;
    QString pilotName;
    QString coPilotName;
    QString gliderType;
    // Aircraft registration
    QString gliderId;

    // Fix timestamps
    QDateTime startDateTimeUtc;
    QTime previousTime;
    QTime currentTime;
    QDateTime currentDateTimeUtc;

    // The track data may contain data with identical timestamps, so we first read
    // all track data into this vector and only then "upsert" the position data
    std::vector<TrackItem> trackData;
    FlightAugmentation flightAugmentation;

    QRegularExpression hRecordDateRegExp;
    QRegularExpression hRecordPilotRegExp;
    QRegularExpression hRecordCoPilotRegExp;
    QRegularExpression hRecordGliderTypeRegExp;
    QRegularExpression hRecordGliderIdRegExp;
    QRegularExpression cRecordTaskDefinitionRegExp;
    QRegularExpression cRecordTaskRegExp;
    QRegularExpression bRecordRegExp;

    static inline const QTime DayChangeThreshold {1, 0, 0, 0};
};

// PUBLIC

IGCImportPlugin::IGCImportPlugin() noexcept
    : d(std::make_unique<IGCImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("IGCImportPlugin::IGCImportPlugin: PLUGIN LOADED");
#endif
}

IGCImportPlugin::~IGCImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("IGCImportPlugin::~IGCImportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

bool IGCImportPlugin::readFile(QFile &file) noexcept
{
    d->file = &file;
    // Manufacturer / identifier
    bool ok = readManufacturer();
    if (ok) {
        // Header
        ok = readRecords();
    }
    return ok;
}

QDateTime IGCImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->startDateTimeUtc;
}

void IGCImportPlugin::updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept
{
    aircraftInfo.tailNumber = d->gliderId;
    aircraftInfo.flightNumber = d->flightNumber;
}

void IGCImportPlugin::updateFlight() noexcept
{
    updateFlightInfo();
    updateFlightCondition();
}

// PRIVATE

bool IGCImportPlugin::readManufacturer() noexcept
{
    const QByteArray line = d->file->readLine();
    return !line.isEmpty() && line.at(0) == ARecord;
}

bool IGCImportPlugin::readRecords() noexcept
{
    bool ok = true;
    QByteArray line = d->file->readLine();
    while (ok && !line.isEmpty()) {
        switch (line.at(0)) {
        case HRecord:
            // Header
            ok = parseHeader(line);
            break;
        case CRecord:
            // Header
            ok = parseTask(line);
            break;
        case BRecord:
            // Fix
            ok = parseFix(line);
            break;
        default:
            // Ignore other record types
            break;
        }
        line = d->file->readLine();
    }
    return ok;
}

bool IGCImportPlugin::parseHeader(const QByteArray &line) noexcept
{
    bool ok = true;
#ifdef DEBUG
    qDebug("H (header) record: %s", qPrintable(line));
#endif

    const QByteArray type = line.mid(2, 3);
    if (type == ::TLCDate) {
        ok = parseHeaderDate(line);
    } else if (type == ::TLCPilot) {
        ok = parseHeaderPilot(line);
    } else if (type == ::TLCCoPilot) {
        ok = parseHeaderCoPilot(line);
    } else if (type == ::TLCGliderType) {
        ok = parseHeaderGliderType(line);
    } else if (type == ::TLCGliderId) {
        ok = parseHeaderGliderId(line);
    }

    return ok;
}

bool IGCImportPlugin::parseHeaderDate(const QByteArray &line) noexcept
{
    bool ok = true;
    QRegularExpressionMatch match = d->hRecordDateRegExp.match(line);
    if (match.hasMatch()) {
        const QStringList captures = match.capturedTexts();
        int year;
        QString yearText = captures.at(::HRecordYearIndex);
        if (yearText.at(0) == '8' || yearText.at(0) == '9') {
            // The glorious 80ies and 90ies: two-digit year dates were all the rage!
            // (The IGC format was invented in the 80ies, so any date starting with
            // either 8 or 9 is boldly assumed to be in those decades)
            year= 1900 + yearText.toInt();
        } else {
            // This code needs fixing again in the year 2080 onwards ;)
            year = 2000 + yearText.toInt();
        }
        const int month = captures.at(::HRecordMonthIndex).toInt();
        const int day   = captures.at(::HRecordDayIndex).toInt();
        d->flightDate.setDate(year, month, day);
        // The flight number is optional
        if (::HRecordFlightNumberIndex < captures.count()) {
            d->flightNumber = captures.at(::HRecordFlightNumberIndex);
        } else {
            // Assume first flight of day
            d->flightNumber = QString("1");
        }

        ok = true;
    } else {
        // No pattern match
        ok = false;
    }

    return ok;
}

bool IGCImportPlugin::parseHeaderText(const QByteArray &line, const QRegularExpression &regExp, QString &text) noexcept
{
    bool ok = true;
    QRegularExpressionMatch match = regExp.match(line);
    if (match.hasMatch()) {
        const QStringList captures = match.capturedTexts();
        // Ignore the data source for now (F: flight recorder, O: observer, P: pilot)
        text = captures.at(2).trimmed().replace('_', ' ');
        ok = true;
    } else {
        // No pattern match
        ok = false;
    }

    return ok;
}

bool IGCImportPlugin::parseHeaderPilot(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordPilotRegExp, d->pilotName);
}

bool IGCImportPlugin::parseHeaderCoPilot(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordCoPilotRegExp, d->coPilotName);
}

bool IGCImportPlugin::parseHeaderGliderType(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordGliderTypeRegExp, d->gliderType);
}

bool IGCImportPlugin::parseHeaderGliderId(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordGliderIdRegExp, d->gliderId);
}

bool IGCImportPlugin::parseTask(const QByteArray &line) noexcept
{
    bool ok;
#ifdef DEBUG
    qDebug("C (task) record: %s", qPrintable(line));
#endif
    QRegularExpressionMatch match = d->cRecordTaskDefinitionRegExp.match(line);
    if (match.hasMatch()) {
        const QStringList captures = match.capturedTexts();
        ok = true;
    } else {
        match = d->cRecordTaskRegExp.match(line);

        if (match.hasMatch()) {
            const QStringList captures = match.capturedTexts();

            // Latitude
            QString degreesText = captures.at(::CRecordLatitudeDegreesIndex);
            QString minutesBy1000Text = captures.at(::CRecordLatitudeMinutesIndex);
            double latitude = parseCoordinate(degreesText, minutesBy1000Text);
            if (captures.at(::CRecordLatitudeDirectionIndex) == ::DirectionTypeSouth) {
                latitude = -latitude;
            }

            // Longitude
            degreesText = captures.at(::CRecordLongitudeDegreesIndex);
            minutesBy1000Text = captures.at(::CRecordLongitudeMinutesIndex);
            double longitude = parseCoordinate(degreesText, minutesBy1000Text);
            if (captures.at(::CRecordLongitudeDirectionIndex) == ::DirectionTypeWest) {
                longitude = -longitude;
            }

            Flight &flight = Logbook::getInstance().getCurrentFlight();
            Waypoint waypoint;

            waypoint.latitude = latitude;
            waypoint.longitude = longitude;
            waypoint.identifier = captures.at(::CRecordTaskIndex);
            waypoint.timestamp = d->currentWaypointTimestamp;
            // The actual timestamps of the waypoints are later updated
            // with the flight duration, after the B records with the timestamps have been
            // read
            ++d->currentWaypointTimestamp;
            flight.getUserAircraft().getFlightPlan().add(std::move(waypoint));
            ok = true;
        } else {
            ok = false;
        }
    }

    return ok;
}

bool IGCImportPlugin::parseFix(const QByteArray &line) noexcept
{
    bool ok;
#ifdef DEBUG
    qDebug("B (fix) record: %s", qPrintable(line));
#endif
    QRegularExpressionMatch match = d->bRecordRegExp.match(line);
    if (match.hasMatch()) {
        const QStringList captures = match.capturedTexts();

        // Timestamp
        const QString timeText = captures.at(::BRecordDateIndex);
        d->currentTime = QTime::fromString(timeText, ::DateFormat);
        if (d->startDateTimeUtc.isNull()) {
            d->startDateTimeUtc = QDateTime(d->flightDate, d->currentTime, QTimeZone::utc());
            d->currentDateTimeUtc = d->startDateTimeUtc;
        } else {
            if (d->currentTime.addSecs(DayChangeThresholdSeconds) < d->previousTime) {
                // Flight crossed "midnight" (next day)
                d->flightDate = d->flightDate.addDays(1);
            }
            d->currentDateTimeUtc = QDateTime(d->flightDate, d->currentTime, QTimeZone::utc());
        }
        d->previousTime = d->currentTime;

        if (d->currentDateTimeUtc.isValid()) {
            const qint64 timestamp = d->startDateTimeUtc.msecsTo(d->currentDateTimeUtc);

            // Latitude
            QString degreesText = captures.at(::BRecordLatitudeDegreesIndex);
            QString minutesBy1000Text = captures.at(::BRecordLatitudeMinutesIndex);
            double latitude = parseCoordinate(degreesText, minutesBy1000Text);
            if (captures.at(::BRecordLatitudeDirectionIndex) == ::DirectionTypeSouth) {
                latitude = -latitude;
            }

            // Longitude
            degreesText = captures.at(::BRecordLongitudeDegreesIndex);
            minutesBy1000Text = captures.at(::BRecordLongitudeMinutesIndex);
            double longitude = parseCoordinate(degreesText, minutesBy1000Text);
            if (captures.at(::BRecordLongitudeDirectionIndex) == ::DirectionTypeWest) {
                longitude = -longitude;
            }

            // GNSS altitude
            const QString altitudeText = captures.at(::BRecordGNSSAltitudeIndex);
            double altitude = Convert::metersToFeet(altitudeText.toDouble());

            TrackItem trackItem = std::make_tuple(timestamp, latitude, longitude, altitude);
            d->trackData.push_back(std::move(trackItem));
            ok = true;
        } else {
            // Invalid timestamp
            ok = false;
        }
    } else {
        // No pattern match
        ok = false;
    }

    return ok;
}

inline double IGCImportPlugin::parseCoordinate(QStringView degreesText, QStringView minutesBy1000Text)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    int degrees = degreesText.toString().toInt();
    double minutes = minutesBy1000Text.toString().toDouble() / 1000.0;
#else
    int degrees = degreesText.toInt();
    double minutes = minutesBy1000Text.toDouble() / 1000.0;
#endif
    return Convert::dm2dd(degrees, minutes);
}

void IGCImportPlugin::updateFlightInfo() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();

    const QString title = d->gliderType;
    flight.setTitle(title);
    const QString description = tr("Glider type:") % " " % d->gliderType % "\n" %
                                tr("Pilot:") % " " % d->pilotName % "\n" %
                                tr("Co-Pilot:") % " " % d->coPilotName % "\n" %
                                tr("Flight date:") % " " % d->unit.formatDate(d->flightDate) % "\n\n" %
                                tr("Aircraft imported on %1 from file: %2").arg(d->unit.formatDateTime(QDateTime::currentDateTime()), d->file->fileName());
    flight.setDescription(description);
    flight.setCreationDate(QFileInfo(*d->file).birthTime());
}

void IGCImportPlugin::updateFlightCondition() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    FlightCondition flightCondition;

    flightCondition.startLocalTime = d->startDateTimeUtc.toLocalTime();
    flightCondition.startZuluTime = d->startDateTimeUtc;
    flightCondition.endLocalTime = d->currentDateTimeUtc.toLocalTime();
    flightCondition.endZuluTime = d->currentDateTimeUtc;

    flight.setFlightCondition(flightCondition);
}

