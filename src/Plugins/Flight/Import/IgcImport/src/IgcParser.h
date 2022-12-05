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
#ifndef IGCPARSER_H
#define IGCPARSER_H

#include <memory>
#include <utility>
#include <vector>
#include <cstdint>

#include <QStringView>
#include <QString>
#include <QLatin1String>
#include <QDate>

class QFile;
class QByteArray;

struct IgcParserPrivate;

/*!
 * Parses flight recorder files in the IGC (International Gliding Association) format.
 *
 * Implemenation note: only fields relevant for Sky Dolly are stored in the result structure.
 */
class IgcParser
{
public:
    IgcParser() noexcept;
    ~IgcParser();

    // "H" record
    using Header = struct
    {
        // Note: we also store the UTC time in this field; the time
        // coming from the first B record ("fix")
        QDateTime flightDateTimeUtc;
        // In addition we store the end date time as well
        QDateTime flightEndDateTimeUtc;
        QString flightNumber;
        QString pilotName;
        QString coPilotName;
        QString gliderType;
        // Aircraft registration
        QString gliderId;
    };

    // "C" record
    using TaskItem = struct Task_
    {
        Task_(float lat, float lon, QString desc)
            : latitude(lat),
              longitude(lon),
              description(std::move(desc))
        {}

        float latitude;
        float longitude;
        QString description;
    };

    using Task = struct
    {
        QDate declarationDate;
        std::vector<TaskItem> tasks;
    };

    /*!
     * The "B record" contains the position and altitude values ("fixes").
     * Note that the environmental noise level ("ENL") is an option addition.
     * If not present then the value is set to 0.0.
     */
    using Fix = struct Fix_
    {
        Fix_(std::int64_t theTimestamp, double lat, double lon, double pressureAlt, double gnssAlt, double enl)
            : timestamp(theTimestamp),
              latitude(lat),
              longitude(lon),
              pressureAltitude(pressureAlt),
              gnssAltitude(gnssAlt),
              environmentalNoiseLevel(enl)
        {}

        // Note: we store a timestamp instead of UTC time here:
        // msecs from the header.flightDateTime
        std::int64_t timestamp;
        double latitude;
        double longitude;
        // Note: in feet
        double pressureAltitude;
        double gnssAltitude;
        /*! Normalised environmental noise level [0.0, 1.0]; 0.0 if not present */
        double environmentalNoiseLevel;
    };

    bool parse(QFile &file) noexcept;
    const Header &getHeader() const noexcept;
    const Task &getTask() const noexcept;
    const std::vector<Fix> &getFixes() const noexcept;
    bool hasEnvironmentalNoiseLevel() const noexcept;

private:
    const std::unique_ptr<IgcParserPrivate> d;

    void init() noexcept;

    // A record, containing manufacturer ID
    bool readManufacturer() noexcept;
    // All records
    bool readRecords() noexcept;

    bool parseHeader(const QByteArray &line) noexcept;
    bool parseHeaderDate(const QByteArray &line) noexcept;
    bool parseHeaderText(const QByteArray &line, const QRegularExpression &regExp, QString &text) noexcept;
    bool parseHeaderPilot(const QByteArray &line) noexcept;
    bool parseHeaderCoPilot(const QByteArray &line) noexcept;
    bool parseHeaderGliderType(const QByteArray &line) noexcept;
    bool parseHeaderGliderId(const QByteArray &line) noexcept;
    bool parseFixAdditions(const QByteArray &line) noexcept;
    bool parseTask(const QByteArray &line) noexcept;
    bool parseFix(const QByteArray &line) noexcept;
    inline double parseCoordinate(QStringView degreesText, QStringView minutesBy1000Text) noexcept;

    // Environmental noise level
    static inline const QLatin1String EnvironmentalNoiseLevel {"ENL"};
};

#endif // IGCPARSER_H
