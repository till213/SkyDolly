/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

#include <QString>
#include <QTimeZone>
#include <QDateTime>
#include <QXmlStreamReader>

#include <Kernel/Convert.h>
#include <Model/Flight.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Waypoint.h>
#include "Kml.h"
#include "AbstractKmlParser.h"

struct AbstractKmlParserPrivate
{
    QXmlStreamReader *xml {nullptr};
};

// PUBLIC

AbstractKmlParser::AbstractKmlParser() noexcept
    : d {std::make_unique<AbstractKmlParserPrivate>()}
{}

AbstractKmlParser::~AbstractKmlParser() = default;

// PROTECTED

void AbstractKmlParser::initialise(QXmlStreamReader *xml) noexcept
{
    d->xml = xml;
}

QXmlStreamReader *AbstractKmlParser::getXmlStreamReader() const noexcept
{
    return d->xml;
}

std::vector<FlightData> AbstractKmlParser::parseKML() noexcept
{
    std::vector<FlightData> flights;
    FlightData flightData;
    flightData.addUserAircraft();
    while (d->xml->readNextStartElement()) {
        const QStringView xmlName = d->xml->name();
        if (xmlName == Kml::Document) {
            parseDocument(flightData);
        }  else if (xmlName == Kml::Folder) {
            parseFolder(flightData);
        } else if (xmlName == Kml::Placemark) {
            parsePlacemark(flightData);
        } else {
            d->xml->skipCurrentElement();
        }
    }
    flights.push_back(std::move(flightData));
    return flights;
}

void AbstractKmlParser::parseDocument(FlightData &flightData) noexcept
{
    while (d->xml->readNextStartElement()) {
        const QStringView xmlName = d->xml->name();
#ifdef DEBUG
        qDebug() << "AbstractKmlParser::parseDocument: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::name) {
            parseDocumentName(flightData);
        } else if (xmlName == Kml::Placemark) {
            parsePlacemark(flightData);
        } else if (xmlName == Kml::Folder) {
            parseFolder(flightData);
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void AbstractKmlParser::parseFolder(FlightData &flightData) noexcept
{
    while (d->xml->readNextStartElement()) {
        const QStringView xmlName = d->xml->name();
#ifdef DEBUG
        qDebug() << "AbstractKmlParser::parseFolder: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::Placemark) {
            parsePlacemark(flightData);
        } else if (xmlName == Kml::Folder) {
            parseFolder(flightData);
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void AbstractKmlParser::parsePlacemark(FlightData &flightData) noexcept
{
    while (d->xml->readNextStartElement()) {
        const QStringView xmlName = d->xml->name();
#ifdef DEBUG
        qDebug() << "AbstractKmlParser::parsePlacemark: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::Track) {
            parseTrack(flightData);
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void AbstractKmlParser::parseDocumentName(FlightData &flightData) noexcept
{
    flightData.title = d->xml->readElementText();
}
