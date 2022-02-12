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
#include <unordered_map>

#include <QFile>
#include <QByteArray>
#include <QStringView>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimeZone>

#include "../../../../../Kernel/src/Convert.h"
#include "IGCParser.h"

// Useful resources:
// - https://regex101.com/
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
    constexpr char IRecord = 'I';
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
    constexpr char HRecordPilotPattern[] = "^H(\\w)PLT(?:.{0,}?:(.*)|(.*))\r\n";
    constexpr char HRecordCoPilotPattern[] = "^H(\\w)CM2(?:.{0,}?:(.*)|(.*))\r\n";
    constexpr char HRecordGliderTypePattern[] = "^H(\\w)GTY(?:.{0,}?:(.*)|(.*))\r\n";
    constexpr char HRecordGliderIdPattern[] = "^H(\\w)GID(?:.{0,}?:(.*)|(.*))\r\n";

    constexpr int HRecordDayIndex = 1;
    constexpr int HRecordMonthIndex = 2;
    constexpr int HRecordYearIndex = 3;
    constexpr int HRecordFlightNumberIndex = 4;

    // I (addition definition) record
    constexpr char IRecordPattern[] = "^I(\\d{2})((?:\\d{4}[A-Z]{3})*)\r\n";
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
    constexpr char BRecordPattern[] = "^B(\\d{6})(\\d{2})(\\d{5})([NS])(\\d{3})(\\d{5})([EW])([AV])(-\\d{4}|\\d{5})(-\\d{4}|\\d{5})(.*)\r\n";
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
    // Optional additions
    constexpr int BRecordAdditionsIndex = 11;
    // The number of regexp captures - the last captured value (additions) is optional
    constexpr int BRecordNofCaptures = 12;
    // Basic data length (bytes)
    constexpr int BRecordBasicDataLength = 35 ;

    // Values
    constexpr char DirectionTypeNorth = 'N';
    constexpr char DirectionTypeSouth = 'S';
    constexpr char DirectionTypeEast = 'E';
    constexpr char DirectionTypeWest = 'W';
}

class IGCParserPrivate
{
public:
    IGCParserPrivate() noexcept
        : file(nullptr),
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

    IGCParser::Header header;
    IGCParser::Task task;
    std::vector<IGCParser::Fix> fixes;

    std::vector<IGCParser::AdditionDefinition> additionDefinitions;

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

IGCParser::IGCParser() noexcept
    : d(std::make_unique<IGCParserPrivate>())
{}

IGCParser::~IGCParser() noexcept
{}

bool IGCParser::parse(QFile &file) noexcept
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
            const qint64 endTimestamp = d->fixes.back().timestamp;
            d->header.flightEndDateTimeUtc = d->header.flightDateTimeUtc.addMSecs(endTimestamp);
        } else {
            d->header.flightEndDateTimeUtc = d->header.flightDateTimeUtc;
        }
    }

    return ok;
}

const IGCParser::Header &IGCParser::getHeader() const noexcept
{
    return d->header;
}

const IGCParser::Task &IGCParser::getTask() const noexcept
{
    return d->task;
}

const std::vector<IGCParser::Fix> &IGCParser::getFixes() const noexcept
{
    return d->fixes;
}

// PRIVATE

void IGCParser::init() noexcept
{
    d->task.tasks.clear();
    d->additionDefinitions.clear();
    d->fixes.clear();
}

bool IGCParser::readManufacturer() noexcept
{
    const QByteArray line = d->file->readLine();
    return !line.isEmpty() && line.at(0) == ARecord;
}

bool IGCParser::readRecords() noexcept
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

bool IGCParser::parseHeader(const QByteArray &line) noexcept
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

bool IGCParser::parseHeaderDate(const QByteArray &line) noexcept
{
    bool ok = true;
    QRegularExpressionMatch match = d->hRecordDateRegExp.match(line);
    if (match.hasMatch()) {
        const QStringList captures = match.capturedTexts();
        int year;
        const QString &yearText = captures.at(::HRecordYearIndex);
        if (yearText.at(0) == '8' || yearText.at(0) == '9') {
            // The glorious 80ies and 90ies: two-digit year dates were all the rage!
            // (The IGC format was invented in the 80ies, so any date starting with
            // either 8 or 9 is boldly assumed to be in those decades)
            year= 1900 + yearText.toInt();
        } else {
            // This code needs fixing again in the year 2080 onwards.
            // Sorry, my future fellows - but not my fault ¯\_(ツ)_/¯
            year = 2000 + yearText.toInt();
        }
        const int month = captures.at(::HRecordMonthIndex).toInt();
        const int day   = captures.at(::HRecordDayIndex).toInt();
        d->header.flightDateTimeUtc.setDate(QDate(year, month, day));
        d->header.flightDateTimeUtc.setTimeZone(QTimeZone::utc());
        // The flight number is optional
        if (::HRecordFlightNumberIndex < captures.count()) {
            d->header.flightNumber = captures.at(::HRecordFlightNumberIndex);
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

bool IGCParser::parseHeaderText(const QByteArray &line, const QRegularExpression &regExp, QString &text) noexcept
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

bool IGCParser::parseHeaderPilot(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordPilotRegExp, d->header.pilotName);
}

bool IGCParser::parseHeaderCoPilot(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordCoPilotRegExp, d->header.coPilotName);
}

bool IGCParser::parseHeaderGliderType(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordGliderTypeRegExp, d->header.gliderType);
}

bool IGCParser::parseHeaderGliderId(const QByteArray &line) noexcept
{
    return parseHeaderText(line, d->hRecordGliderIdRegExp, d->header.gliderId);
}

bool IGCParser::parseFixAdditions(const QByteArray &line) noexcept
{
    bool ok;
    QRegularExpressionMatch match = d->iRecordRegExp.match(line);
    if (match.hasMatch()) {
        const QStringList captures = match.capturedTexts();
        const int nofAdditions = captures.at(::IRecordNofAdditionsIndex).toInt();
        const QString &definitions = captures.at(::IRecordAdditionsDefinitionsIndex);

        // Validate the number of bytes: each definition is expected to be
        // of the form SS FF CCC (7 bytes in total)
        if (definitions.length() >= nofAdditions * ::IRecordAdditionDefinitionLength) {
            int index = 0;
            for (int i = 0; i < nofAdditions; ++i) {
                const QStringRef ref = definitions.midRef(i * ::IRecordAdditionDefinitionLength, ::IRecordAdditionDefinitionLength);
                int startOffset = ref.mid(0, 2).toInt();
                int endOffset = ref.mid(2, 2).toInt();
                const ThreeLetterCode tlc = ref.mid(4, 3).toLatin1();
                d->additionDefinitions.emplace_back(tlc, startOffset, endOffset);
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

bool IGCParser::parseTask(const QByteArray &line) noexcept
{
    bool ok;
    QRegularExpressionMatch match = d->cRecordTaskDefinitionRegExp.match(line);
    if (match.hasMatch()) {
        const QStringList captures = match.capturedTexts();
        ok = true;
    } else {
        match = d->cRecordTaskRegExp.match(line);

        if (match.hasMatch()) {
            const QStringList captures = match.capturedTexts();

            // Latitude
            const QString &latitudeText = captures.at(::CRecordLatitudeDegreesIndex);
            QString minutesBy1000Text = captures.at(::CRecordLatitudeMinutesIndex);
            double latitude = parseCoordinate(latitudeText, minutesBy1000Text);
            if (captures.at(::CRecordLatitudeDirectionIndex) == ::DirectionTypeSouth) {
                latitude = -latitude;
            }

            // Longitude
            const QString &longitudeText  = captures.at(::CRecordLongitudeDegreesIndex);
            minutesBy1000Text = captures.at(::CRecordLongitudeMinutesIndex);
            double longitude = parseCoordinate(longitudeText, minutesBy1000Text);
            if (captures.at(::CRecordLongitudeDirectionIndex) == ::DirectionTypeWest) {
                longitude = -longitude;
            }

            d->task.tasks.emplace_back(latitude, longitude, captures.at(::CRecordTaskIndex));

            ok = true;
        } else {
            ok = false;
        }
    }
    return ok;
}

bool IGCParser::parseFix(const QByteArray &line) noexcept
{
    bool ok;
    QRegularExpressionMatch match = d->bRecordRegExp.match(line);
    if (match.hasMatch()) {
        const QStringList captures = match.capturedTexts();

        // Timestamp
        const QString &timeText = captures.at(::BRecordDateIndex);
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

            const qint64 timestamp = d->header.flightDateTimeUtc.msecsTo(d->currentDateTimeUtc);

            // Latitude
            const QString &latitudeText = captures.at(::BRecordLatitudeDegreesIndex);
            const QString &latitudeMinutesBy1000Text = captures.at(::BRecordLatitudeMinutesIndex);
            double latitude = parseCoordinate(latitudeText, latitudeMinutesBy1000Text);
            if (captures.at(::BRecordLatitudeDirectionIndex) == ::DirectionTypeSouth) {
                latitude = -latitude;
            }

            // Longitude
            const QString &longitudeTex = captures.at(::BRecordLongitudeDegreesIndex);
            const QString &longitudeMinutesBy1000Text = captures.at(::BRecordLongitudeMinutesIndex);
            double longitude = parseCoordinate(longitudeTex, longitudeMinutesBy1000Text);
            if (captures.at(::BRecordLongitudeDirectionIndex) == ::DirectionTypeWest) {
                longitude = -longitude;
            }

            // Pressure altitude
            const QString &pressureAltitudeText = captures.at(::BRecordPressureAltitudeIndex);
            const double pressureAltitude = Convert::metersToFeet(pressureAltitudeText.toDouble());

            // GNSS altitude
            const QString &gnssAltitudeText = captures.at(::BRecordGNSSAltitudeIndex);
            const double gnssAltitude = Convert::metersToFeet(gnssAltitudeText.toDouble());

            d->fixes.emplace_back(timestamp, latitude, longitude, pressureAltitude, gnssAltitude);

            // Optional additions
            if (d->additionDefinitions.size() > 0) {
                // But mandatory if specified in I records
                if (captures.length() == ::BRecordNofCaptures) {
                    ok = parseAdditions(captures.at(::BRecordAdditionsIndex), d->fixes.back().additions);
                } else {
                    ok = false;
                }
            } else {
                ok = true;
            }
            if (ok) {
                ;
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

inline double IGCParser::parseCoordinate(QStringView degreesText, QStringView minutesBy1000Text) noexcept
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

inline bool IGCParser::parseAdditions(QStringView additionValues, Additions &additions) noexcept
{
    for (auto &def : d->additionDefinitions) {
        const int len = def.endOffset - def.startOffset;
        const QString addition = additionValues.mid(def.startOffset - ::BRecordBasicDataLength, len).toString();
        additions.insert(std::make_pair(def.name, addition));
    }
    return true;
}
