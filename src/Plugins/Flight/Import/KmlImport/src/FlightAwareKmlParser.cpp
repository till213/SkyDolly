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
#include <QXmlStreamReader>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Convert.h>
#include <Model/FlightData.h>
#include <Model/Aircraft.h>
#include <Model/FlightPlan.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Waypoint.h>
#include "Kml.h"
#include "AbstractKmlTrackParser.h"
#include "FlightAwareKmlParser.h"

// PUBLIC

FlightAwareKmlParser::FlightAwareKmlParser() noexcept
    : AbstractKmlTrackParser()
{}

FlightAwareKmlParser::~FlightAwareKmlParser() = default;

// FlightAware KML files (are expected to) have 3 Placemarks, with:
// - <Point> Takeoff airpart
// - <Point> Destination airport
// - <gx:Track> timestamps (<when>) and positions (<gx:coord>)
std::vector<FlightData> FlightAwareKmlParser::parse(QXmlStreamReader &xmlStreamReader) noexcept
{
    initialise(&xmlStreamReader);
    std::vector<FlightData> flights = parseKML();
    enrichFlightData(flights);
    return flights;
}

// PRIVATE

void FlightAwareKmlParser::parsePlacemark(FlightData &flightData) noexcept
{
    auto *xml = getXmlStreamReader();
    QString placemarkName;
    while (xml->readNextStartElement()) {
        const auto xmlName = xml->name();
#ifdef DEBUG
        qDebug() << "FlightAwareKmlParser::parsePlacemark: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::name) {
            placemarkName = xml->readElementText();
            if (placemarkName.endsWith(" Airport")) {
                // Extract the 4 letter ICAO code
                placemarkName = placemarkName.left(4);
            }
        } else if (xmlName == Kml::Point) {
            parseWaypoint(flightData, placemarkName);
        } else if (xmlName == Kml::Track) {
            // The track contains the flight number
            flightData.flightNumber = placemarkName;
            parseTrack(flightData);
        } else {
            xml->skipCurrentElement();
        }
    }
}

void FlightAwareKmlParser::parseWaypoint(FlightData &flightData, QString icaoOrName) noexcept
{
    auto &aircraft = flightData.getUserAircraft();
    auto *xml = getXmlStreamReader();
    bool ok {true};
    while (xml->readNextStartElement()) {
        const QStringView xmlName = xml->name();
#ifdef DEBUG
        qDebug() << "FlightAwareKmlParser::parseWaypoint: XML start element:" << xmlName.toString();
#endif
        if (xmlName == QStringLiteral("coordinates")) {
            const auto coordinatesText = xml->readElementText();
            const auto coordinates = coordinatesText.split(",");
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

                aircraft.getFlightPlan().add(std::move(waypoint));
            } else {
                xml->raiseError("Invalid GPS coordinate.");
            }
        } else {
            xml->skipCurrentElement();
        }
    }
}

void FlightAwareKmlParser::enrichFlightData(std::vector<FlightData> &flights) noexcept
{
    for (auto &flightData : flights) {
        flightData.creationTime = getFirstDateTimeUtc();
        for (auto &aircraft : flightData) {
            updateAircraftWaypoints(aircraft);
        }
    }
}

void FlightAwareKmlParser::updateAircraftWaypoints(Aircraft &aircraft) noexcept
{
    std::size_t positionCount = aircraft.getPosition().count();
    if (positionCount > 0) {
        std::size_t waypointCount = aircraft.getFlightPlan().count();
        if (waypointCount > 0) {

            const auto &position = aircraft.getPosition();
            const auto &firstPositionData = position.getFirst();
            const auto &lastPositionData = position.getLast();
            const auto startDateTimeUtc = getFirstDateTimeUtc();
            const auto endDateTimeUtc = startDateTimeUtc.addMSecs(lastPositionData.timestamp);

            auto &departure = aircraft.getFlightPlan()[0];
            departure.timestamp = firstPositionData.timestamp;
            departure.altitude = static_cast<float>(firstPositionData.altitude);
            departure.localTime = getFirstDateTimeUtc().toLocalTime();
            departure.zuluTime = getFirstDateTimeUtc();

            if (waypointCount > 1) {
                const PositionData &lastPositionData = aircraft.getPosition().getLast();
                Waypoint &arrival = aircraft.getFlightPlan()[1];
                arrival.timestamp = lastPositionData.timestamp;
                arrival.altitude = static_cast<float>(lastPositionData.altitude);
                arrival.localTime = endDateTimeUtc.toLocalTime();
                arrival.zuluTime = endDateTimeUtc;
            }
        }
    } else {
        // No positions - use timestamps 0, 1, 2, ...
        std::int64_t currentWaypointTimestamp {0};
        for (Waypoint &waypoint : aircraft.getFlightPlan()) {
            waypoint.timestamp = currentWaypointTimestamp;
            ++currentWaypointTimestamp;
        }
    }
}
