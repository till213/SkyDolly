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
#include <cstdint>

#include <QString>
#include <QStringLiteral>
#include <QCoreApplication>
#include <QTimeZone>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QRegularExpression>

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Waypoint.h"
#include "FlightRadar24KMLParser.h"

class FlightRadar24KMLParserPrivate
{
public:
    typedef struct {
        int64_t timestamp;
        double latitude;
        double longitude;
        double altitude;
        int speed;
        int heading;
    } TrackItem;
    // The track data may contain data with identical timestamps
    std::vector<TrackItem> trackData;

    FlightRadar24KMLParserPrivate(QXmlStreamReader &xmlStreamReader) noexcept
        : xml(xmlStreamReader),
          currentWaypointTimestamp(0),
          speedRegExp(FlightRadar24KMLParserPrivate::SpeedPattern),
          headingRegExp(FlightRadar24KMLParserPrivate::HeadingPattern)
    {}

    QXmlStreamReader &xml;
    QString flightNumber;
    int64_t currentWaypointTimestamp;
    QDateTime firstDateTimeUtc;
    QDateTime currentDateTimeUtc;

    QRegularExpression speedRegExp;
    QRegularExpression headingRegExp;

private:
    static inline const QString SpeedPattern {QStringLiteral("<b>Speed:<\\/b><\\/span> <span>(\\d+) kt<\\/span>")};
    static inline const QString HeadingPattern {QStringLiteral("<b>Heading:<\\/b><\\/span> <span>(\\d+)&deg;<\\/span>")};
};

// PUBLIC

FlightRadar24KMLParser::FlightRadar24KMLParser(QXmlStreamReader &xmlStreamReader) noexcept
    : d(std::make_unique<FlightRadar24KMLParserPrivate>(xmlStreamReader))
{
#ifdef DEBUG
    qDebug("FlightRadar24KMLParser::~FlightRadar24KMLParser: CREATED");
#endif
}

FlightRadar24KMLParser::~FlightRadar24KMLParser() noexcept
{
#ifdef DEBUG
    qDebug("FlightRadar24KMLParser::~FlightRadar24KMLParser: DELETED");
#endif
}

// FlightRadar24 KML files (are expected to) have 3 Placemarks, with:
// - <Point> Takeoff airpart
// - <Point> Destination airport
// - <gx:Track> timestamps (<when>) and positions (<gx:coord>)s
void FlightRadar24KMLParser::parse(QDateTime &firstDateTimeUtc, QDateTime &lastDateTimeUtc, QString &flightNumber) noexcept
{
    d->trackData.clear();

    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
        if (xmlName == QStringLiteral("Folder")) {
            parseFolder();
        } else {
            d->xml.skipCurrentElement();
        }
    }

    // Now "upsert" the position data, taking duplicate timestamps into account
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Position &position = flight.getUserAircraft().getPosition();
    for (const FlightRadar24KMLParserPrivate::TrackItem &trackItem : d->trackData) {
        PositionData positionData;
        positionData.timestamp = trackItem.timestamp;
        positionData.latitude = trackItem.latitude;
        positionData.longitude = trackItem.longitude;
        positionData.altitude = trackItem.altitude;
        positionData.velocityBodyZ = trackItem.speed;
        positionData.heading = trackItem.heading;

        position.upsertLast(std::move(positionData));
    }

    // Set timezone
    d->firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    d->currentDateTimeUtc.setTimeZone(QTimeZone::utc());

    firstDateTimeUtc = d->firstDateTimeUtc;
    lastDateTimeUtc = d->currentDateTimeUtc;
    flightNumber = d->flightNumber;
}

// PRIVATE

void FlightRadar24KMLParser::parseFolder() noexcept
{
    QString name;
    bool routePlacemark = false;
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("FlightRadar24KMLParser::readDocument: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == QStringLiteral("name")) {
            name = d->xml.readElementText();
            if (name == QStringLiteral("Route")) {
                routePlacemark = true;
            }
        } else if (xmlName == QStringLiteral("Placemark") && routePlacemark) {
            // We are interested in the "Route" placemark (only)
            parsePlacemark();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void FlightRadar24KMLParser::parsePlacemark() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("FlightRadar24KMLParser::readDocument: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == QStringLiteral("description")) {
            if (!parseDescription()) {
                d->xml.raiseError(QStringLiteral("Could not parse description text."));
            }
        } else if (xmlName == QStringLiteral("TimeStamp")) {
            parseTimestamp();
        } else if (xmlName == QStringLiteral("Point")) {
            parsePoint();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

bool FlightRadar24KMLParser::parseDescription() noexcept
{
    bool ok;
    const QString description = d->xml.readElementText();
#ifdef DEBUG
        qDebug("FlightRadar24KMLParser::parseDescription: description %s", qPrintable(description));
#endif
    QRegularExpressionMatch match = d->speedRegExp.match(description);
    if (match.hasMatch()) {
        FlightRadar24KMLParserPrivate::TrackItem trackItem;;


        trackItem.speed = match.captured(1).toInt();
        const int pos = match.capturedEnd();
        match = d->headingRegExp.match(description, pos);
        if (match.hasMatch()) {
            trackItem.heading = match.captured(1).toInt();
            d->trackData.push_back(std::move(trackItem));
            ok = true;
        } else {
            ok = false;
        }
    } else {
        ok = false;
    }

    return ok;
}

void FlightRadar24KMLParser::parseTimestamp() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("FlightRadar24KMLParser::parseTimestamp: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == QStringLiteral("when")) {
            const QString dateTimeText = d->xml.readElementText();
            if (d->firstDateTimeUtc.isNull()) {
                d->firstDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
                d->currentDateTimeUtc = d->firstDateTimeUtc;
            } else {
                d->currentDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
            }
            if (d->currentDateTimeUtc.isValid()) {
                (d->trackData.back()).timestamp = d->firstDateTimeUtc.msecsTo(d->currentDateTimeUtc);
            } else {
                d->xml.raiseError(QStringLiteral("Invalid timestamp."));
            }
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void FlightRadar24KMLParser::parsePoint() noexcept
{
    bool ok = true;
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("FlightRadar24KMLParser::parsePoint: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == QStringLiteral("coordinates")) {
            const QString coordinatesText = d->xml.readElementText();
            const QStringList coordinates = coordinatesText.split(",");
            if (coordinates.count() == 3) {

                const double longitude = coordinates.at(0).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(QStringLiteral("Invalid longitude number."));
                }
                const double latitude = coordinates.at(1).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(QStringLiteral("Invalid latitude number."));
                }
                const double altitude = coordinates.at(2).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(QStringLiteral("Invalid altitude number."));
                }
                if (ok) {
                    auto &trackItem = d->trackData.back();
                    trackItem.latitude = latitude;
                    trackItem.longitude = longitude;
                    trackItem.altitude = Convert::metersToFeet(altitude);
                }

            } else {
                d->xml.raiseError(QStringLiteral("Invalid GPS coordinate."));
            }
        } else {
            d->xml.skipCurrentElement();
        }
    }
}
