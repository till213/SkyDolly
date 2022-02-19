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

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Waypoint.h"
#include "FlightAwareKMLParser.h"

class FlightAwareKMLParserPrivate
{
public:
    FlightAwareKMLParserPrivate(QXmlStreamReader &xmlStreamReader) noexcept
        : xml(xmlStreamReader),
          currentWaypointTimestamp(0)
    {}

    QXmlStreamReader &xml;
    QString flightNumber;
    std::int64_t currentWaypointTimestamp;
    QDateTime firstDateTimeUtc;
    QDateTime currentDateTimeUtc;
};

// PUBLIC

FlightAwareKMLParser::FlightAwareKMLParser(QXmlStreamReader &xmlStreamReader) noexcept
    : d(std::make_unique<FlightAwareKMLParserPrivate>(xmlStreamReader))
{
#ifdef DEBUG
    qDebug("FlightAwareKMLParser::~FlightAwareKMLParser: CREATED");
#endif
}

FlightAwareKMLParser::~FlightAwareKMLParser() noexcept
{
#ifdef DEBUG
    qDebug("FlightAwareKMLParser::~FlightAwareKMLParser: DELETED");
#endif
}

// FlightAware KML files (are expected to) have 3 Placemarks, with:
// - <Point> Takeoff airpart
// - <Point> Destination airport
// - <gx:Track> timestamps (<when>) and positions (<gx:coord>)
void FlightAwareKMLParser::parse(QDateTime &firstDateTimeUtc, QDateTime &lastDateTimeUtc, QString &flightNumber) noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
        if (xmlName == QStringLiteral("Placemark")) {
            parsePlacemark();
        } else {
            d->xml.skipCurrentElement();
        }
    }
    firstDateTimeUtc = d->firstDateTimeUtc;
    lastDateTimeUtc = d->currentDateTimeUtc;
    flightNumber = d->flightNumber;
}

// PRIVATE

void FlightAwareKMLParser::parsePlacemark() noexcept
{
    QString name;
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("FlightAwareKMLParser::parsePlacemark: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == QStringLiteral("name")) {
            name = d->xml.readElementText();
            if (name.endsWith(QStringLiteral(" Airport"))) {
                // Extract the 4 letter ICAO code
                name = name.left(4);
            }
        } else if (xmlName == QStringLiteral("Point")) {
            parseWaypoint(name);
        } else if (xmlName == QStringLiteral("Track")) {
            // The track contains the flight number
            d->flightNumber = name;
            parseTrack();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void FlightAwareKMLParser::parseWaypoint(const QString &icaoOrName) noexcept
{
    bool ok;
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("FlightAwareKMLParser::parseWaypoint: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == QStringLiteral("coordinates")) {
            const QString coordinatesText = d->xml.readElementText();
            const QStringList coordinates = coordinatesText.split(",");
            if (coordinates.count() == 3) {
                Waypoint waypoint;
                waypoint.longitude = coordinates.at(0).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(QStringLiteral("Invalid longitude number."));
                }
                waypoint.latitude = coordinates.at(1).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(QStringLiteral("Invalid latitude number."));
                }
                waypoint.altitude = coordinates.at(2).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(QStringLiteral("Invalid altitude number."));
                }
                waypoint.identifier = icaoOrName;
                waypoint.timestamp = d->currentWaypointTimestamp;
                // The actual timestamps of the waypoints are later updated
                // with the flight duration, once the entire gx:Track data
                // has been parsed
                ++d->currentWaypointTimestamp;

                Flight &flight = Logbook::getInstance().getCurrentFlight();
                flight.getUserAircraft().getFlightPlan().add(std::move(waypoint));
            } else {
                d->xml.raiseError(QStringLiteral("Invalid GPS coordinate."));
            }
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void FlightAwareKMLParser::parseTrack() noexcept
{
    // Timestamp (msec), latitude (degrees), longitude (degrees), altitude (feet)
    typedef std::tuple<std::int64_t, double, double, double> TrackItem;
    // The track data may contain data with identical timestamps, so we first read
    // all track data into this vector and only then "upsert" the position data
    std::vector<TrackItem> trackData;

    bool ok = true;
    int currentTrackDataIndex = 0;
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("FlightAwareKMLParser::parseTrack: XML start element: %s", qPrintable(xmlName.toString()));
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
                const std::int64_t timestamp = d->firstDateTimeUtc.msecsTo(d->currentDateTimeUtc);
                TrackItem trackItem = std::make_tuple(timestamp, 0.0, 0.0, 0.0);
                trackData.push_back(std::move(trackItem));
            } else {
                d->xml.raiseError(QStringLiteral("Invalid timestamp."));
            }
        } else if (xmlName == QStringLiteral("coord")) {
            const QString coordinatesText = d->xml.readElementText();
            const QStringList coordinates = coordinatesText.split(" ");
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
                    std::get<1>(trackData[currentTrackDataIndex]) = latitude;
                    std::get<2>(trackData[currentTrackDataIndex]) = longitude;
                    std::get<3>(trackData[currentTrackDataIndex]) = Convert::metersToFeet(altitude);
                    ++currentTrackDataIndex;
                }

            } else {
                d->xml.raiseError(QStringLiteral("Invalid GPS coordinate."));
            }
        } else {
            d->xml.skipCurrentElement();
        }
    }

    // Now "upsert" the position data, taking duplicate timestamps into account
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Position &position = flight.getUserAircraft().getPosition();
    for (const TrackItem &trackItem : trackData) {
        PositionData positionData;
        positionData.timestamp = std::get<0>(trackItem);
        positionData.latitude = std::get<1>(trackItem);
        positionData.longitude = std::get<2>(trackItem);
        positionData.altitude = std::get<3>(trackItem);

        position.upsertLast(std::move(positionData));
    }

    // Set timezone
    d->firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    d->currentDateTimeUtc.setTimeZone(QTimeZone::utc());
}
