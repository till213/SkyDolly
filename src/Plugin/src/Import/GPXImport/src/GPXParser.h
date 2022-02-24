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
#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <memory>

#include <QDateTime>
#include <QString>

class QXmlStreamReader;
class QDateTime;

class GPXImportSettings;
class GPXParserPrivate;

class GPXParser
{
public:
    GPXParser(QXmlStreamReader &xmlStreamReader, const GPXImportSettings &importSettings) noexcept;
    virtual ~GPXParser() noexcept;

    virtual void parse() noexcept;
    QDateTime getFirstDateTimeUtc() const noexcept;
    QString getDocumentName() const noexcept;
    QString getDescription() const noexcept;

private:
    std::unique_ptr<GPXParserPrivate> d;

    void parseGPX() noexcept;
    void parseMetadata() noexcept;
    void parseWaypoint() noexcept;
    void parseRoute() noexcept;
    void parseRoutePoint() noexcept;
    void parseTrack() noexcept;
    void parseTrackSegment() noexcept;
    inline void parseTrackPoint() noexcept;
    inline bool parseWaypointType(double &latitude, double &longitude, double &altitude, QString &identifier, QDateTime &dateTime) noexcept;
};

#endif // GPXPARSER_H
