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
#include <memory>
#include <cstdint>

#include <QString>
#include <QStringLiteral>
#include <QTimeZone>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QRegularExpression>

#include <Kernel/Convert.h>
#include <Model/Flight.h>
#include <Model/FlightPlan.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Waypoint.h>
#include "Kml.h"
#include "FlightRadar24KmlParser.h"

struct FlightRadar24KmlParserPrivate
{
    using TrackItem = struct {
        std::int64_t timestamp;
        double latitude;
        double longitude;
        double altitude;
        int speed;
        int heading;
    };
    // The track data may contain data with identical timestamps
    std::vector<TrackItem> trackData;

    FlightRadar24KmlParserPrivate() noexcept
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    Flight *flight {nullptr};
    QXmlStreamReader *xml {nullptr};
    QString documentName;
    QString flightNumber;
    QDateTime firstDateTimeUtc;

    static const QRegularExpression speedRegExp;
    static const QRegularExpression headingRegExp;

private:
    static inline const QString SpeedPattern {QStringLiteral(R"(<b>Speed:<\/b><\/span> <span>(\d+) kt<\/span>)")};
    static inline const QString HeadingPattern {QStringLiteral(R"(<b>Heading:<\/b><\/span> <span>(\d+)&deg;<\/span>)")};
};

const QRegularExpression FlightRadar24KmlParserPrivate::speedRegExp {FlightRadar24KmlParserPrivate::SpeedPattern};
const QRegularExpression FlightRadar24KmlParserPrivate::headingRegExp {FlightRadar24KmlParserPrivate::HeadingPattern};

// PUBLIC

FlightRadar24KmlParser::FlightRadar24KmlParser() noexcept
    : d(std::make_unique<FlightRadar24KmlParserPrivate>())
{}

FlightRadar24KmlParser::~FlightRadar24KmlParser() = default;

// FlightRadar24 KML files (are expected to) have one "Route" folder with <Placemarks> containing
// - <description> - HTML snippet containing speed and heading
// - <Timestamp> timestamps
// - <Point> - the coordinates of the track
void FlightRadar24KmlParser::parse(QXmlStreamReader &xmlStreamReader, Flight &flight) noexcept
{
    d->flight = &flight;
    d->xml = &xmlStreamReader;
    d->trackData.clear();

    if (d->xml->readNextStartElement()) {
#ifdef DEBUG
        qDebug() << "FlightRadar24KmlParser::readKML: XML start element:" << d->xml->name().toString();
#endif
        if (d->xml->name() == Kml::Document) {
            parseName();
            parseDocument();
        } else {
            d->xml->raiseError("The file is not a KML document.");
        }
    } else {
        d->xml->raiseError("Error reading the XML data.");
    }

    // Now "upsert" the position data, taking duplicate timestamps into account
    Position &position = d->flight->getUserAircraft().getPosition();
    for (const FlightRadar24KmlParserPrivate::TrackItem &trackItem : d->trackData) {
        PositionData positionData {trackItem.latitude, trackItem.longitude, trackItem.altitude};
        positionData.timestamp = trackItem.timestamp;
        positionData.velocityBodyZ = trackItem.speed;
        positionData.trueHeading = trackItem.heading;

        position.upsertLast(positionData);
    }
}

QString FlightRadar24KmlParser::getDocumentName() const noexcept
{
    return d->documentName;
}

QString FlightRadar24KmlParser::getFlightNumber() const noexcept
{
    return d->flightNumber;
}

QDateTime FlightRadar24KmlParser::getFirstDateTimeUtc() const noexcept
{
    return d->firstDateTimeUtc;
}

// PRIVATE

void FlightRadar24KmlParser::parseName() noexcept
{
    if (d->xml->readNextStartElement()) {
#ifdef DEBUG
        qDebug() << "FlightAwareKmlParser::readDocument: XML start element:" << d->xml->name().toString();
#endif
        if (d->xml->name() == Kml::name) {
            d->documentName = d->xml->readElementText();
        } else {
            d->xml->raiseError("The KML document does not have a name element.");
        }
    }
}

void FlightRadar24KmlParser::parseDocument() noexcept
{
    while (d->xml->readNextStartElement()) {
        const QStringRef xmlName = d->xml->name();
        if (xmlName == Kml::Folder) {
            parseFolder();
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void FlightRadar24KmlParser::parseFolder() noexcept
{
    QString name;
    bool routePlacemark = false;
    while (d->xml->readNextStartElement()) {
        const QStringRef xmlName = d->xml->name();
#ifdef DEBUG
        qDebug() << "FlightRadar24KmlParser::readDocument: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::name) {
            name = d->xml->readElementText();
            if (name == QStringLiteral("Route")) {
                routePlacemark = true;
            }
        } else if (xmlName == Kml::Placemark && routePlacemark) {
            // We are interested in the "Route" placemark (only)
            parsePlacemark();
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void FlightRadar24KmlParser::parsePlacemark() noexcept
{
    while (d->xml->readNextStartElement()) {
        const QStringRef xmlName = d->xml->name();
#ifdef DEBUG
        qDebug() << "FlightRadar24KmlParser::readDocument: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::description) {
            if (!parseDescription()) {
                d->xml->raiseError("Could not parse description text.");
            }
        } else if (xmlName == Kml::TimeStamp) {
            parseTimestamp();
        } else if (xmlName == Kml::Point) {
            parsePoint();
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

bool FlightRadar24KmlParser::parseDescription() noexcept
{
    bool ok {false};
    const QString description = d->xml->readElementText();
#ifdef DEBUG
    qDebug() << "FlightRadar24KmlParser::parseDescription: description:" << description;
#endif
    QRegularExpressionMatch match = d->speedRegExp.match(description);
    if (match.hasMatch()) {
        FlightRadar24KmlParserPrivate::TrackItem trackItem;
        trackItem.speed = match.captured(1).toInt();
        const int pos = match.capturedEnd();
        match = d->headingRegExp.match(description, pos);
        if (match.hasMatch()) {
            trackItem.heading = match.captured(1).toInt();
            d->trackData.push_back(trackItem);
            ok = true;
        } else {
            ok = false;
        }
    }

    return ok;
}

void FlightRadar24KmlParser::parseTimestamp() noexcept
{
    QDateTime currentDateTimeUtc;
    currentDateTimeUtc.setTimeZone(QTimeZone::utc());

    while (d->xml->readNextStartElement()) {
        const QStringRef xmlName = d->xml->name();
#ifdef DEBUG
        qDebug() << "FlightRadar24KmlParser::parseTimestamp: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::when) {
            const QString dateTimeText = d->xml->readElementText();
            if (d->firstDateTimeUtc.isNull()) {
                d->firstDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
                currentDateTimeUtc = d->firstDateTimeUtc;
            } else {
                currentDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
            }
            if (currentDateTimeUtc.isValid()) {
                (d->trackData.back()).timestamp = d->firstDateTimeUtc.msecsTo(currentDateTimeUtc);
            } else {
                d->xml->raiseError("Invalid timestamp.");
            }
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void FlightRadar24KmlParser::parsePoint() noexcept
{
    bool ok {true};
    while (d->xml->readNextStartElement()) {
        const QStringRef xmlName = d->xml->name();
#ifdef DEBUG
        qDebug() << "FlightRadar24KmlParser::parsePoint: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::coordinates) {
            const QString coordinatesText = d->xml->readElementText();
            const QStringList coordinates = coordinatesText.split(",");
            if (coordinates.count() == 3) {

                const double longitude = coordinates.at(0).toDouble(&ok);
                if (!ok) {
                    d->xml->raiseError("Invalid longitude number.");
                }
                const double latitude = coordinates.at(1).toDouble(&ok);
                if (!ok) {
                    d->xml->raiseError("Invalid latitude number.");
                }
                const double altitude = coordinates.at(2).toDouble(&ok);
                if (!ok) {
                    d->xml->raiseError("Invalid altitude number.");
                }
                if (ok) {
                    auto &trackItem = d->trackData.back();
                    trackItem.latitude = latitude;
                    trackItem.longitude = longitude;
                    trackItem.altitude = Convert::metersToFeet(altitude);
                }

            } else {
                d->xml->raiseError("Invalid GPS coordinate.");
            }
        } else {
            d->xml->skipCurrentElement();
        }
    }
}
