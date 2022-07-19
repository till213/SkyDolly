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

#include <Kernel/Convert.h>
#include <Model/Flight.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Waypoint.h>
#include "Kml.h"
#include "AbstractKmlParser.h"

class AbstractKmlParserPrivate
{
public:
    AbstractKmlParserPrivate() noexcept
        : flight(nullptr),
          xml(nullptr)
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    Flight *flight;
    QXmlStreamReader *xml;
    QString documentName;    
    QDateTime firstDateTimeUtc;
};

// PUBLIC

AbstractKmlParser::AbstractKmlParser() noexcept
    : d(std::make_unique<AbstractKmlParserPrivate>())
{
#ifdef DEBUG
    qDebug("AbstractKmlParser::AbstractKmlParser: CREATED");
#endif
}

AbstractKmlParser::~AbstractKmlParser() noexcept
{
#ifdef DEBUG
    qDebug("AbstractKmlParser::~AbstractKmlParser: DELETED");
#endif
}

QString AbstractKmlParser::getDocumentName() const noexcept
{
    return d->documentName;
}

// PROTECTED

void AbstractKmlParser::initialise(Flight *flight, QXmlStreamReader *xml) noexcept
{
    d->flight = flight;
    d->xml = xml;
}

Flight *AbstractKmlParser::getFlight() const noexcept
{
    return d->flight;
}

QXmlStreamReader *AbstractKmlParser::getXmlStreamReader() const noexcept
{
    return d->xml;
}

void AbstractKmlParser::parseKML() noexcept
{
    while (d->xml->readNextStartElement()) {
        const QStringRef xmlName = d->xml->name();
        if (xmlName == Kml::Document) {
            parseDocument();
        }  else if (xmlName == Kml::Folder) {
            parseFolder();
        } else if (xmlName == Kml::Placemark) {
            parsePlacemark();
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void AbstractKmlParser::parseDocument() noexcept
{
    while (d->xml->readNextStartElement()) {
        const QStringRef xmlName = d->xml->name();
#ifdef DEBUG
        qDebug("AbstractKmlParser::parseDocument: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == Kml::name) {
            parseDocumentName();
        } else if (xmlName == Kml::Placemark) {
            parsePlacemark();
        } else if (xmlName == Kml::Folder) {
            parseFolder();
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void AbstractKmlParser::parseFolder() noexcept
{
    while (d->xml->readNextStartElement()) {
        const QStringRef xmlName = d->xml->name();
#ifdef DEBUG
        qDebug("AbstractKmlParser::parseFolder: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == Kml::Placemark) {
            parsePlacemark();
        } else if (xmlName == Kml::Folder) {
            parseFolder();
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void AbstractKmlParser::parsePlacemark() noexcept
{
    while (d->xml->readNextStartElement()) {
        const QStringRef xmlName = d->xml->name();
#ifdef DEBUG
        qDebug("AbstractKmlParser::parsePlacemark: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == Kml::Track) {
            parseTrack();
        } else {
            d->xml->skipCurrentElement();
        }
    }
}

void AbstractKmlParser::parseDocumentName() noexcept
{
    d->documentName = d->xml->readElementText();
}
