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
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Waypoint.h"
#include "KML.h"
#include "AbstractKMLParser.h"

class AbstractKMLParserPrivate
{
public:
    AbstractKMLParserPrivate(QXmlStreamReader &xmlStreamReader) noexcept
        : xml(xmlStreamReader)
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    QString documentName;
    QXmlStreamReader &xml;
    QDateTime firstDateTimeUtc;
};

// PUBLIC

AbstractKMLParser::AbstractKMLParser(QXmlStreamReader &xmlStreamReader) noexcept
    : d(std::make_unique<AbstractKMLParserPrivate>(xmlStreamReader))
{
#ifdef DEBUG
    qDebug("AbstractKMLParser::~AbstractKMLParser: CREATED");
#endif
}

AbstractKMLParser::~AbstractKMLParser() noexcept
{
#ifdef DEBUG
    qDebug("AbstractKMLParser::~AbstractKMLParser: DELETED");
#endif
}

QString AbstractKMLParser::getDocumentName() const noexcept
{
    return d->documentName;
}

// PROTECTED

void AbstractKMLParser::parseKML() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
        if (xmlName == KML::Document) {
            parseDocument();
        }  else if (xmlName == KML::Folder) {
            parseFolder();
        } else if (xmlName == KML::Placemark) {
            parsePlacemark();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void AbstractKMLParser::parseDocument() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("AbstractKMLParser::parseDocument: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == KML::name) {
            parseDocumentName();
        } else if (xmlName == KML::Placemark) {
            parsePlacemark();
        } else if (xmlName == KML::Folder) {
            parseFolder();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void AbstractKMLParser::parseFolder() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("AbstractKMLParser::parseFolder: XML start element: %s", qPrintable(xmlName.toString()));
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

void AbstractKMLParser::parsePlacemark() noexcept
{
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("AbstractKMLParser::parsePlacemark: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == KML::Track) {
            parseTrack();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void AbstractKMLParser::parseDocumentName() noexcept
{
    d->documentName = d->xml.readElementText();
}
