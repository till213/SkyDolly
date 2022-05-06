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
#include <tuple>
#include <cmath>
#include <cstdint>

#include <GeographicLib/DMS.hpp>

#include <QFile>
#include <QByteArray>
#include <QStringView>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimeZone>

#include <Kernel/Convert.h>
#include "IgcParser.h"

// Useful resources:
// - https://regex101.com/
namespace
{
    // Timestamp (msec), latitude (degrees), longitude (degrees), altitude (feet)
    typedef std::tuple<std::int64_t, double, double, double> TrackItem;

    // One hour threshold
    constexpr int DayChangeThresholdSeconds = 60 * 60;

    // Record types
    constexpr char ARecord = 'A';
    constexpr char HRecord = 'H';
    constexpr char CRecord = 'C';
    constexpr char IRecord = 'I';
    constexpr char BRecord = 'B';

    // Three letter codes (TLC)
    constexpr char TLCDate[] = "DTE";
    constexpr char TLCPilot[] = "PLT";
    constexpr char TLCCoPilot[] = "CM2";
    constexpr char TLCGliderType[] = "GTY";
    constexpr char TLCGliderId[] = "GID";

    // Offsets
    constexpr int InvalidOffset = -1;

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

    // I (addition definition) record
    constexpr char IRecordPattern[] = "^[I](\\d{2})((?:\\d{4}[A-Z]{3})+)";
    constexpr int IRecordNofAdditionsIndex = 1;
    constexpr int IRecordAdditionsDefinitionsIndex = 2;
    // Length of addition definition [bytes]
    constexpr int IRecordAdditionDefinitionLength = 7;

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
    constexpr int BRecordPressureAltitudeIndex = 9;
    // GNSS altitude (in metres, above the WGS84 ellipsoid)
    constexpr int BRecordGNSSAltitudeIndex = 10;

    // Values
    constexpr char DirectionTypeNorth = 'N';
    constexpr char DirectionTypeSouth = 'S';
    constexpr char DirectionTypeEast = 'E';
    constexpr char DirectionTypeWest = 'W';
}

class IgcParserPrivate
{
public:
    IgcParserPrivate() noexcept
        : file(nullptr),
          enlAddition(false),
          enlStartOffset(::InvalidOffset),
          enlLength(0),
          hRecordDateRegExp(QString(::HRecordDatePattern)),
          hRecordPilotRegExp(QString(::HRecordPilotPattern)),
          hRecordCoPilotRegExp(QString(::HRecordCoPilotPattern)),
          hRecordGliderTypeRegExp(QString(::HRecordGliderTypePattern)),
          hRecordGliderIdRegExp(QString(::HRecordGliderIdPattern)),
          iRecordRegExp(QString(::IRecordPattern)),
          cRecordTaskDefinitionRegExp(QString(::CRecordTaskDefinitionPattern)),
          cRecordTaskRegExp(QString(::CRecordTaskPattern)),          
          bRecordRegExp(QString(::BRecordPattern))
    {}

    QFile *file;

    // Fix timestamps
    QTime previousTime;
    QDateTime currentDateTimeUtc;

    IgcParser::Header header;
    IgcParser::Task task;
    // Altitude in meters
    std::vector<IgcParser::Fix> fixes;

    bool enlAddition;
    int enlStartOffset;
    int enlLength;
    double maxEnlValue;

    QRegularExpression hRecordDateRegExp;
    QRegularExpression hRecordPilotRegExp;
    QRegularExpression hRecordCoPilotRegExp;
    QRegularExpression hRecordGliderTypeRegExp;
    QRegularExpression hRecordGliderIdRegExp;
    QRegularExpression iRecordRegExp;
    QRegularExpression cRecordTaskDefinitionRegExp;
    QRegularExpression cRecordTaskRegExp;    
    QRegularExpression bRecordRegExp;
};

// PUBLIC

IgcParser::IgcParser() noexcept
    : d(std::make_unique<IgcParserPrivate>())
{}

IgcParser::~IgcParser() noexcept
{}

bool IgcParser::parse(QFile &file) noexcept
{
    init();

    d->file = &file;
    // Manufacturer / identifier
    bool ok = readManufacturer();
    if (ok) {
        ok = readRecords();
    }
    if (ok) {
        if (d->fixes.size() > 0) {
            const std::int64_t endTimestamp = d->fixes.back().timestamp;
            d->header.flightEndDateTimeUtc = d->header.flightDateTimeUtc.addMSecs(endTimestamp);
        } else {
            d->header.flightEndDateTimeUtc = d->header.flightDateTimeUtc;
        }
    }

    return ok;
}

const IgcParser::Header &IgcParser::getHeader() const noexcept
{
    return d->header;
}

const IgcParser::Task &IgcParser::getTask() const noexcept
{
    return d->task;
}

const std::vector<IgcParser::Fix> &IgcParser::getFixes() const noexcept
{
    return d->fixes;
}

bool IgcParser::hasEnvironmentalNoiseLevel() const noexcept
{
    return d->enlAddition;
}

// PRIVATE

void IgcParser::init() noexcept
{
    d->enlAddition = false;
    d->task.tasks.clear();
    d->fixes.clear();
}

bool IgcParser::readManufacturer() noexcept
{
    const QByteArray line = d->file->readLine();
    return !line.isEmpty() && line.at(0) == ARecord;
}

bool IgcParser::readRecords() noexcept
{
    bool ok = true;
    QByteArray line = d->file->readLine();
    while (ok && !line.isEmpty()) {
        switch (line.at(0)) {
        case HRecord:
            // Header
            ok = parseHeader(line);
            break;
        case IRecord:
            // Header
            ok = parseFixAdditions(line);
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

bool IgcParser::parseHeader(const QByteArray &line) noexcept
{
    bool ok = true;

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

bool IgcParser::parseHeaderDate(const QByteArray &line) noexcept
{
    bool ok = true;
    QRegularExpressionMatch match = d->hRecordDateRegExp.match(line);
    if (match.hasMatch()) {
        int year;
        const QStringView yearText = match.capturedView(::HRecordYearIndex);
        if (yearText.at(0) == '8' || yearText.at(0) == '9') {
            // The glorious 80ies and 90ies: two-digit year dates were all the rage!
            // (The IGC format was invented in the 80ies, so any date starting with
            // either 8 or 9 is boldly assumed to be in those decades)
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            year= 1900 + yearText.toString().toInt();
#else
            year= 1900 + yearText.toInt();
#endif
        } else {
            // This code needs fixing again in the year 2080 onwards.
            // Sorry, my future fellows - but not my fault ¯\_(ツ)_/¯
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            year = 2000 + yearText.toString().toInt();
#else
            year = 2000 + yearText.toInt();
#endif
        }
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        const int month = match.capturedView(::HRecordMonthIndex).toString().toInt();
        const int day   = match.capturedView(::HRecordDayIndex).toString().toInt();
#else
        const int month = match.capturedView(::HRecordMonthIndex).toInt();
        const int day   = match.capturedView(::HRecordDayIndex).toInt();
#endif
        d->header.flightDateTimeUtc.setDate(QDate(year, month, day));
        d->header.flightDateTimeUtc.setTimeZone(QTimeZone::utc());
        // The flight number is optional
        if (::HRecordFlightNumberIndex == match.lastCapturedIndex()) {
            d->header.flightNumber = match.captured(::HRecordFlightNumberIndex);
        } else {
            // Assume first flight of day
            d->header.flightNumber = QString("1");
        }

        ok = true;
    } else {
        // No pattern match
        ok = false;
    }
    return ok;
}

bool IgcParser::parseHeaderText(const QByteArray &line, const QRegularExpression &regExp, QString &text) noexcept
{
    bool ok = true;
    QRegularExpressionMatch match = regExp.match(line);
    if (match.hasMatch()) {
        // Ignore the data source for now (F: flight recorder, O: observer, P: pilot)
        text = match.captured(2).trimmed().replace('_', ' ');
        ok = true;
    } else {
        // No pattern match
        ok = false;
    }
    return ok;
}

bool IgcParser::parseHeaderPilot(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordPilotRegExp, d->header.pilotName);
}

bool IgcParser::parseHeaderCoPilot(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordCoPilotRegExp, d->header.coPilotName);
}

bool IgcParser::parseHeaderGliderType(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordGliderTypeRegExp, d->header.gliderType);
}

bool IgcParser::parseHeaderGliderId(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordGliderIdRegExp, d->header.gliderId);
}

bool IgcParser::parseFixAdditions(const QByteArray &line) noexcept
{
    bool ok;
    QRegularExpressionMatch match = d->iRecordRegExp.match(line);
    if (match.hasMatch()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        const int nofAdditions = match.capturedView(::IRecordNofAdditionsIndex).toString().toInt();
#else
        const int nofAdditions = match.capturedView(::IRecordNofAdditionsIndex).toInt();
#endif
        const QStringView definitions = match.capturedView(::IRecordAdditionsDefinitionsIndex);
        // Validate the number of bytes: each definition is expected to be
        // of the form SS FF CCC (7 bytes in total)
        if (definitions.length() >= nofAdditions * ::IRecordAdditionDefinitionLength) {
            int index = 0;
            for (int i = 0; i < nofAdditions; ++i) {
                const QStringView def = definitions.mid(i * ::IRecordAdditionDefinitionLength, ::IRecordAdditionDefinitionLength);
                // We are only interested in the ENL addition for now
                if (def.mid(4, 3) == EnvironmentalNoiseLevel) {
                    d->enlAddition = true;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                    d->enlStartOffset = def.mid(0, 2).toString().toInt() - 1;
                    d->enlLength = def.mid(2, 2).toString().toInt() - d->enlStartOffset;
#else
                    d->enlStartOffset = def.mid(0, 2).toInt() - 1;
                    d->enlLength = def.mid(2, 2).toInt() - d->enlStartOffset;
#endif
                    d->maxEnlValue = ::pow(10, d->enlLength ) - 1;
                }
            }
            ok = true;
        } else {
            ok = false;
        }
    } else {
        // No pattern match
        ok = false;
    }
    return ok;
}

bool IgcParser::parseTask(const QByteArray &line) noexcept
{
    bool ok;
    QRegularExpressionMatch match = d->cRecordTaskDefinitionRegExp.match(line);
    if (match.hasMatch()) {
        ok = true;
    } else {
        match = d->cRecordTaskRegExp.match(line);
        if (match.hasMatch()) {
            // Latitude
            const QStringView latitudeText = match.capturedView(::CRecordLatitudeDegreesIndex);
            QStringView minutesBy1000Text = match.capturedView(::CRecordLatitudeMinutesIndex);
            double latitude = parseCoordinate(latitudeText, minutesBy1000Text);
            if (match.capturedView(::CRecordLatitudeDirectionIndex) == ::DirectionTypeSouth) {
                latitude = -latitude;
            }

            // Longitude
            const QStringView longitudeText  = match.capturedView(::CRecordLongitudeDegreesIndex);
            minutesBy1000Text = match.capturedView(::CRecordLongitudeMinutesIndex);
            double longitude = parseCoordinate(longitudeText, minutesBy1000Text);
            if (match.capturedView(::CRecordLongitudeDirectionIndex) == ::DirectionTypeWest) {
                longitude = -longitude;
            }

            d->task.tasks.emplace_back(latitude, longitude, match.captured(::CRecordTaskIndex));

            ok = true;
        } else {
            ok = false;
        }
    }
    return ok;
}

bool IgcParser::parseFix(const QByteArray &line) noexcept
{
    bool ok;
    QRegularExpressionMatch match = d->bRecordRegExp.match(line);
    if (match.hasMatch()) {

        // Timestamp
        const QString timeText = match.captured(::BRecordDateIndex);
        const QTime currentTime = QTime::fromString(timeText, ::DateFormat);
        if (d->fixes.size() > 0) {
            if (currentTime.addSecs(DayChangeThresholdSeconds) < d->previousTime) {
                // Flight crossed "midnight" (next day)
                d->currentDateTimeUtc.setDate(d->currentDateTimeUtc.date().addDays(1));
            }
            d->currentDateTimeUtc.setTime(currentTime);
        } else {
            // First fix
            d->header.flightDateTimeUtc.setTime(currentTime);
            d->currentDateTimeUtc = d->header.flightDateTimeUtc;
        }
        d->previousTime = currentTime;

        if (d->currentDateTimeUtc.isValid()) {

            const std::int64_t timestamp = d->header.flightDateTimeUtc.msecsTo(d->currentDateTimeUtc);

            // Latitude
            const QStringView latitudeText = match.capturedView(::BRecordLatitudeDegreesIndex);
            const QStringView latitudeMinutesBy1000Text = match.capturedView(::BRecordLatitudeMinutesIndex);
            double latitude = parseCoordinate(latitudeText, latitudeMinutesBy1000Text);
            if (match.capturedView(::BRecordLatitudeDirectionIndex) == ::DirectionTypeSouth) {
                latitude = -latitude;
            }

            // Longitude
            const QStringView longitudeText = match.capturedView(::BRecordLongitudeDegreesIndex);
            const QStringView longitudeMinutesBy1000Text = match.capturedView(::BRecordLongitudeMinutesIndex);
            double longitude = parseCoordinate(longitudeText, longitudeMinutesBy1000Text);
            if (match.capturedView(::BRecordLongitudeDirectionIndex) == ::DirectionTypeWest) {
                longitude = -longitude;
            }

            // Pressure altitude
            const QStringView pressureAltitudeText = match.capturedView(::BRecordPressureAltitudeIndex);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            const double pressureAltitude = pressureAltitudeText.toString().toDouble();
#else
            const double pressureAltitude = pressureAltitudeText.toDouble();
#endif

            // GNSS altitude
            const QStringView gnssAltitudeText = match.capturedView(::BRecordGNSSAltitudeIndex);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
            const double gnssAltitude = gnssAltitudeText.toString().toDouble();
#else
            const double gnssAltitude = gnssAltitudeText.toDouble();
#endif
            // Optional environmental noise level (ENL) addition
            double enlNorm;
            if (d->enlAddition) {
                const QByteArray enlText = line.mid(d->enlStartOffset, d->enlLength);
                const double enlValue = enlText.toDouble(&ok);
                if (ok) {
                    enlNorm = enlValue / d->maxEnlValue;
                }
            } else {
                enlNorm = 0.0;
                ok = true;
            }

            if (ok) {
                d->fixes.emplace_back(timestamp, latitude, longitude, pressureAltitude, gnssAltitude, enlNorm);
            }
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

inline double IgcParser::parseCoordinate(QStringView degreesText, QStringView minutesBy1000Text) noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    int degrees = degreesText.toString().toInt();
    double minutes = minutesBy1000Text.toString().toDouble() / 1000.0;
#else
    int degrees = degreesText.toInt();
    double minutes = minutesBy1000Text.toDouble() / 1000.0;
#endif
    return GeographicLib::DMS::Decode(degrees, minutes);
}
