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
#include <QXmlStreamReader>

#include <Kernel/Convert.h>
#include "../../../../Model/src/Flight.h"
#include "../../../../Model/src/FlightPlan.h"
#include "../../../../Model/src/Position.h"
#include "../../../../Model/src/PositionData.h"
#include "../../../../Model/src/Waypoint.h"
#include "Kml.h"
#include "AbstractKmlTrackParser.h"
#include "FlightAwareKmlParser.h"

class FlightAwareKmlParserPrivate
{
public:
    FlightAwareKmlParserPrivate() noexcept
    {}

    QString flightNumber;
};

// PUBLIC

FlightAwareKmlParser::FlightAwareKmlParser() noexcept
    : AbstractKmlTrackParser(),
      d(std::make_unique<FlightAwareKmlParserPrivate>())
{
#ifdef DEBUG
    qDebug("FlightAwareKmlParser::FlightAwareKmlParser: CREATED");
#endif
}

FlightAwareKmlParser::~FlightAwareKmlParser() noexcept
{
#ifdef DEBUG
    qDebug("FlightAwareKmlParser::~FlightAwareKmlParser: DELETED");
#endif
}

// FlightAware KML files (are expected to) have 3 Placemarks, with:
// - <Point> Takeoff airpart
// - <Point> Destination airport
// - <gx:Track> timestamps (<when>) and positions (<gx:coord>)
void FlightAwareKmlParser::parse(QXmlStreamReader &xmlStreamReader, Flight &flight) noexcept
{
    initialise(&flight, &xmlStreamReader);
    parseKML();
    updateWaypoints();
}

QString FlightAwareKmlParser::getFlightNumber() const noexcept
{
    return d->flightNumber;
}

// PRIVATE

void FlightAwareKmlParser::parsePlacemark() noexcept
{
    QXmlStreamReader *xml = getXmlStreamReader();
    QString placemarkName;
    while (xml->readNextStartElement()) {
        const QStringRef xmlName = xml->name();
#ifdef DEBUG
        qDebug("FlightAwareKmlParser::parsePlacemark: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == Kml::name) {
            placemarkName = xml->readElementText();
            if (placemarkName.endsWith(QStringLiteral(" Airport"))) {
                // Extract the 4 letter ICAO code
                placemarkName = placemarkName.left(4);
            }
        } else if (xmlName == Kml::Point) {
            parseWaypoint(placemarkName);
        } else if (xmlName == Kml::Track) {
            // The track contains the flight number
            d->flightNumber = placemarkName;
            parseTrack();
        } else {
            xml->skipCurrentElement();
        }
    }
}

void FlightAwareKmlParser::parseWaypoint(const QString &icaoOrName) noexcept
{
    Flight *flight = getFlight();
    QXmlStreamReader *xml = getXmlStreamReader();
    bool ok;
    while (xml->readNextStartElement()) {
        const QStringRef xmlName = xml->name();
#ifdef DEBUG
        qDebug("FlightAwareKmlParser::parseWaypoint: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == QStringLiteral("coordinates")) {
            const QString coordinatesText = xml->readElementText();
            const QStringList coordinates = coordinatesText.split(",");
            if (coordinates.count() == 3) {
                Waypoint waypoint;
                waypoint.longitude = coordinates.at(0).toFloat(&ok);
                if (!ok) {
                    xml->raiseError("Invalid longitude number.");
                }
                waypoint.latitude = coordinates.at(1).toFloat(&ok);
                if (!ok) {
                    xml->raiseError("Invalid latitude number.");
                }
                waypoint.altitude = coordinates.at(2).toFloat(&ok);
                if (!ok) {
                    xml->raiseError("Invalid altitude number.");
                }
                waypoint.identifier = icaoOrName;
                // The actual timestamps of the waypoints are later updated
                // in updateWaypoints with the actual timestamp, once the entire
                // gx:Track data has been parsed
                waypoint.timestamp = TimeVariableData::InvalidTime;

                flight->getUserAircraft().getFlightPlan().add(std::move(waypoint));
            } else {
                xml->raiseError("Invalid GPS coordinate.");
            }
        } else {
            xml->skipCurrentElement();
        }
    }
}

void FlightAwareKmlParser::updateWaypoints() noexcept
{
    Aircraft &aircraft = getFlight()->getUserAircraft();

    int positionCount = aircraft.getPosition().count();
    if (positionCount > 0) {
        int waypointCount = aircraft.getFlightPlan().count();
        if (waypointCount > 0) {

            const Position &position = aircraft.getPositionConst();
            const PositionData firstPositionData = position.getFirst();
            const PositionData lastPositionData = position.getLast();
            const QDateTime startDateTimeUtc = getFirstDateTimeUtc();
            const QDateTime endDateTimeUtc = startDateTimeUtc.addMSecs(lastPositionData.timestamp);

            Waypoint &departure = aircraft.getFlightPlan()[0];
            departure.timestamp = firstPositionData.timestamp;
            departure.altitude = firstPositionData.altitude;
            departure.localTime = getFirstDateTimeUtc().toLocalTime();
            departure.zuluTime = getFirstDateTimeUtc();

            if (waypointCount > 1) {
                const PositionData &lastPositionData = aircraft.getPosition().getLast();
                Waypoint &arrival = aircraft.getFlightPlan()[1];
                arrival.timestamp = lastPositionData.timestamp;
                arrival.altitude = lastPositionData.altitude;
                arrival.localTime = endDateTimeUtc.toLocalTime();
                arrival.zuluTime = endDateTimeUtc;
            }
        }
    } else {
        // No positions - use timestamps 0, 1, 2, ...
        std::int64_t currentWaypointTimestamp = 0;
        for (Waypoint &waypoint : aircraft.getFlightPlan()) {
            waypoint.timestamp = currentWaypointTimestamp;
            ++currentWaypointTimestamp;

        }
    }
}
