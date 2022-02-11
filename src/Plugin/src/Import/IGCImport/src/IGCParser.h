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
#ifndef IGCPARSER_H
#define IGCPARSER_H

#include <memory>
#include <vector>

#include <QStringView>
#include <QString>
#include <QDate>

class QFile;
class QByteArray;

class IGCParserPrivate;

/*!
 * Parses flight recorder files in the IGC (International Gliding Association) format.
 *
 * Implemenation note: only fields relevant for Sky Dolly are stored in the result structure.
 */
class IGCParser
{
public:
    IGCParser() noexcept;
    ~IGCParser() noexcept;

    // "H" record
    typedef struct
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
    } Header;

    // "C" record
    typedef struct Task_
    {
        Task_(double lat, double lon, QString desc)
            : latitude(lat),
              longitude(lon),
              description(desc)
        {}

        double latitude;
        double longitude;
        QString description;
    } TaskItem;

    typedef struct
    {
        QDate declarationDate;
        std::vector<TaskItem> tasks;
    } Task;

    // "B" record
    typedef struct Fix_
    {
        Fix_(qint64 theTimestamp, double lat, double lon, double pressureAlt, double gnssAlt)
            : timestamp(theTimestamp),
              latitude(lat),
              longitude(lon),
              pressureAltitude(pressureAlt),
              gnssAltitude(gnssAlt)
        {}

        // Note: we store a timestamp instead of UTC time here:
        // msecs from the header.flightDateTime
        qint64 timestamp;
        double latitude;
        double longitude;
        // Note: in feet
        double pressureAltitude;
        double gnssAltitude;
    } Fix;

    bool parse(QFile &file) noexcept;
    const Header &getHeader() const noexcept;
    const Task &getTask() const noexcept;
    const std::vector<Fix> &getFixes() const noexcept;

private:
    std::unique_ptr<IGCParserPrivate> d;

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

    bool parseTask(const QByteArray &line) noexcept;
    bool parseFix(const QByteArray &line) noexcept;
    inline double parseCoordinate(QStringView degreesText, QStringView minutesBy1000Text);
};

#endif // IGCPARSER_H
