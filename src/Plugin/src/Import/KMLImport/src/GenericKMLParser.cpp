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
#include "AbstractKMLTrackParser.h"
#include "GenericKMLParser.h"

class GenericKMLParserPrivate
{
public:
    GenericKMLParserPrivate(QXmlStreamReader &xmlStreamReader) noexcept
        : xml(xmlStreamReader),
          currentWaypointTimestamp(0)
    {}

    QXmlStreamReader &xml;
    QString documentName;
    QString flightNumber;
    std::int64_t currentWaypointTimestamp;
};

// PUBLIC

GenericKMLParser::GenericKMLParser(QXmlStreamReader &xmlStreamReader) noexcept
    : AbstractKMLTrackParser(xmlStreamReader),
      d(std::make_unique<GenericKMLParserPrivate>(xmlStreamReader))
{
#ifdef DEBUG
    qDebug("GenericKMLParser::~GenericKMLParser: CREATED");
#endif
}

GenericKMLParser::~GenericKMLParser() noexcept
{
#ifdef DEBUG
    qDebug("GenericKMLParser::~GenericKMLParser: DELETED");
#endif
}

// Generic KML files (are expected to) have at least one "gx:Track"
void GenericKMLParser::parse() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
        if (xmlName == KML::Placemark) {
            parsePlacemark();
        } else if (xmlName == KML::Document) {
            parseDocument();
        } else if (xmlName == KML::Folder) {
            parseFolder();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

QString GenericKMLParser::getDocumentName() const noexcept
{
    return d->documentName;
}

QString GenericKMLParser::getFlightNumber() const noexcept
{
    return d->flightNumber;
}

// PRIVATE

void GenericKMLParser::parseDocument() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("GenericKMLParser::parseDocument: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == KML::Placemark) {
            parsePlacemark();
        } else if (xmlName == KML::Folder) {
            parseFolder();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GenericKMLParser::parseFolder() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("GenericKMLParser::parseFolder: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == KML::Placemark) {
            parsePlacemark();
        } else if (xmlName == KML::Folder) {
            parseFolder();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GenericKMLParser::parsePlacemark() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("GenericKMLParser::parsePlacemark: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == KML::Track) {
            parseTrack();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}
