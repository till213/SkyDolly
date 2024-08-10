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
#include <Model/Location.h>
#include <Model/Aircraft.h>
#include <Model/FlightPlan.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Waypoint.h>
#include "Kml.h"
#include "KmlParserIntf.h"
#include "PlacemarkKmlParser.h"

struct PlacemarkKmlParserPrivate
{

};

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

void PlacemarkKmlParser::parsePlacemark(Location &location) noexcept
{
    auto *xml = getXmlStreamReader();
    while (xml->readNextStartElement()) {
        const QStringView xmlName = xml->name();
#ifdef DEBUG
        qDebug() << "PlacemarkKmlParser::parsePlacemark: XML start element:" << xmlName.toString();
#endif
        // TODO IMPLEMENT ME!!!
        //if (xmlName == Kml::Track) {
            //parseTrack(location);
        //} else {
            xml->skipCurrentElement();
        //}
    }
}
