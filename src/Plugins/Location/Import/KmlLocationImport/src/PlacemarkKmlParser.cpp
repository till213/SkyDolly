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
#include <QString>
#include <QStringLiteral>
#include <QXmlStreamReader>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Convert.h>
#include <Model/Location.h>
#include <Model/Aircraft.h>
#include <Model/FlightPlan.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Waypoint.h>
#include "Kml.h"
#include "KmlParserIntf.h"
#include "PlacemarkKmlParser.h"

// PUBLIC

PlacemarkKmlParser::PlacemarkKmlParser() noexcept
    : AbstractKmlParser()
{}

PlacemarkKmlParser::~PlacemarkKmlParser() = default;

// Placemark KML files (are expected to) have at least one Placemark element
std::vector<Location> PlacemarkKmlParser::parse(QXmlStreamReader &xmlStreamReader) noexcept
{
    initialise(&xmlStreamReader);
    std::vector<Location> locations = parseKML();
    return locations;
}

// PROTECTED

void PlacemarkKmlParser::parsePlacemark(std::vector<Location> &locations) noexcept
{
    auto *xml = getXmlStreamReader();
    Location location;
    while (xml->readNextStartElement()) {
        const QStringView xmlName = xml->name();
#ifdef DEBUG
        qDebug() << "PlacemarkKmlParser::parsePlacemark: XML start element:" << xmlName.toString();
#endif        
        if (xmlName == Kml::name) {
            location.title = xml->readElementText();
        } else if (xmlName == Kml::description) {
            location.description = xml->readElementText();
        } else if (xmlName == Kml::Point) {
            parsePoint(location);
        } else {
            xml->skipCurrentElement();
        }
    }
    locations.push_back(std::move(location));
}

// PRIVATE

void PlacemarkKmlParser::parsePoint(Location &location) noexcept
{
    bool ok {true};
    auto *xml = getXmlStreamReader();
    while (xml->readNextStartElement()) {
        const QStringView xmlName = xml->name();
#ifdef DEBUG
        qDebug() << "PlacemarkKmlParser::parsePoint: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::coordinates) {
            const QString coordinatesText = xml->readElementText();
            const QStringList coordinates = coordinatesText.split(",");
            if (coordinates.count() == 3) {

                const double longitude = coordinates.at(0).toDouble(&ok);
                if (!ok) {
                    xml->raiseError("Invalid longitude number.");
                }
                const double latitude = coordinates.at(1).toDouble(&ok);
                if (!ok) {
                    xml->raiseError("Invalid latitude number.");
                }
                const double altitude = coordinates.at(2).toDouble(&ok);
                if (!ok) {
                    xml->raiseError("Invalid altitude number.");
                }
                if (ok) {
                    location.latitude = latitude;
                    location.longitude = longitude;
                    location.altitude = Convert::metersToFeet(altitude);
                }

            } else {
                xml->raiseError("Invalid GPS coordinate.");
            }
        } else {
            xml->skipCurrentElement();
        }
    }
}
