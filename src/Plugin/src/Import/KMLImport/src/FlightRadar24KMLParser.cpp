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
#include "../../../../../Model/src/Waypoint.h"
#include "KML.h"
#include "FlightRadar24KMLParser.h"

class FlightRadar24KMLParserPrivate
{
public:
    typedef struct {
        std::int64_t timestamp;
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
          speedRegExp(FlightRadar24KMLParserPrivate::SpeedPattern),
          headingRegExp(FlightRadar24KMLParserPrivate::HeadingPattern)
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    QXmlStreamReader &xml;
    QString documentName;
    QString flightNumber;
    QDateTime firstDateTimeUtc;

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

// FlightRadar24 KML files (are expected to) have one "Route" folder with <Placemarks> containing
// - <description> - HTML snippet containing speed and heading
// - <Timestamp> timestamps
// - <Point> - the coordinates of the track
void FlightRadar24KMLParser::parse() noexcept
{
    d->trackData.clear();

    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("FlightRadar24KMLParser::readKML: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == KML::Document) {
            parseName();
            parseDocument();
        } else {
            d->xml.raiseError("The file is not a KML document.");
        }
    } else {
        d->xml.raiseError("Error reading the XML data.");
    }

    // Now "upsert" the position data, taking duplicate timestamps into account
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Position &position = flight.getUserAircraft().getPosition();
    for (const FlightRadar24KMLParserPrivate::TrackItem &trackItem : d->trackData) {
        PositionData positionData {trackItem.latitude, trackItem.longitude, trackItem.altitude};
        positionData.timestamp = trackItem.timestamp;
        positionData.velocityBodyZ = trackItem.speed;
        positionData.heading = trackItem.heading;

        position.upsertLast(std::move(positionData));
    }
}

QString FlightRadar24KMLParser::getDocumentName() const noexcept
{
    return d->documentName;
}

QString FlightRadar24KMLParser::getFlightNumber() const noexcept
{
    return d->flightNumber;
}

QDateTime FlightRadar24KMLParser::getFirstDateTimeUtc() const noexcept
{
    return d->firstDateTimeUtc;
}

// PRIVATE

void FlightRadar24KMLParser::parseName() noexcept
{
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("FlightAwareKMLParser::readDocument: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == KML::name) {
            d->documentName = d->xml.readElementText();
        } else {
            d->xml.raiseError("The KML document does not have a name element.");
        }
    }
}

void FlightRadar24KMLParser::parseDocument() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
        if (xmlName == KML::Folder) {
            parseFolder();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void FlightRadar24KMLParser::parseFolder() noexcept
{
    QString name;
    bool routePlacemark = false;
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("FlightRadar24KMLParser::readDocument: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == KML::name) {
            name = d->xml.readElementText();
            if (name == QStringLiteral("Route")) {
                routePlacemark = true;
            }
        } else if (xmlName == KML::Placemark && routePlacemark) {
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
        if (xmlName == KML::description) {
            if (!parseDescription()) {
                d->xml.raiseError("Could not parse description text.");
            }
        } else if (xmlName == KML::TimeStamp) {
            parseTimestamp();
        } else if (xmlName == KML::Point) {
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
        FlightRadar24KMLParserPrivate::TrackItem trackItem;
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
    QDateTime currentDateTimeUtc;
    currentDateTimeUtc.setTimeZone(QTimeZone::utc());

    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("FlightRadar24KMLParser::parseTimestamp: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == KML::when) {
            const QString dateTimeText = d->xml.readElementText();
            if (d->firstDateTimeUtc.isNull()) {
                d->firstDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
                currentDateTimeUtc = d->firstDateTimeUtc;
            } else {
                currentDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
            }
            if (currentDateTimeUtc.isValid()) {
                (d->trackData.back()).timestamp = d->firstDateTimeUtc.msecsTo(currentDateTimeUtc);
            } else {
                d->xml.raiseError("Invalid timestamp.");
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
        if (xmlName == KML::coordinates) {
            const QString coordinatesText = d->xml.readElementText();
            const QStringList coordinates = coordinatesText.split(",");
            if (coordinates.count() == 3) {

                const double longitude = coordinates.at(0).toDouble(&ok);
                if (!ok) {
                    d->xml.raiseError("Invalid longitude number.");
                }
                const double latitude = coordinates.at(1).toDouble(&ok);
                if (!ok) {
                    d->xml.raiseError("Invalid latitude number.");
                }
                const double altitude = coordinates.at(2).toDouble(&ok);
                if (!ok) {
                    d->xml.raiseError("Invalid altitude number.");
                }
                if (ok) {
                    auto &trackItem = d->trackData.back();
                    trackItem.latitude = latitude;
                    trackItem.longitude = longitude;
                    trackItem.altitude = Convert::metersToFeet(altitude);
                }

            } else {
                d->xml.raiseError("Invalid GPS coordinate.");
            }
        } else {
            d->xml.skipCurrentElement();
        }
    }
}
