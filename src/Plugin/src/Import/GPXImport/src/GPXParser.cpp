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

#include <QString>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QTimeZone>

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "GPX.h"
#include "GPXParser.h"

class GPXParserPrivate
{
public:
    GPXParserPrivate(QXmlStreamReader &xmlStreamReader) noexcept
        : xml(xmlStreamReader)
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    QXmlStreamReader &xml;
    QDateTime firstDateTimeUtc;
};

// PUBLIC

GPXParser::GPXParser(QXmlStreamReader &xmlStreamReader) noexcept
    : d(std::make_unique<GPXParserPrivate>(xmlStreamReader))
{
#ifdef DEBUG
    qDebug("GPXParser::~GPXParser: CREATED");
#endif
}

GPXParser::~GPXParser() noexcept
{
#ifdef DEBUG
    qDebug("GPXParser::~GPXParser: DELETED");
#endif
}

void GPXParser::parse() noexcept
{
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GPXParser::parse: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == GPX::gpx) {
            parseGPX();
        } else {
            d->xml.raiseError(QStringLiteral("The file is not a KML file."));
        }
    }
}

void GPXParser::parseGPX() noexcept
{
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GPXParser::parseGPX: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == GPX::metadata) {
            parseMetadata();
        } else if (d->xml.name() == GPX::wpt) {
            parseWaypoint();
        } else if (d->xml.name() == GPX::trk) {
            parseTrack();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GPXParser::parseMetadata() noexcept
{
    // @todo IMPLEMENT ME!!!
    d->xml.skipCurrentElement();
}

void GPXParser::parseWaypoint() noexcept
{
    // @todo IMPLEMENT ME!!!
    d->xml.skipCurrentElement();
}

void GPXParser::parseTrack() noexcept
{
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GPXParser::parseTrack: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == GPX::trkseg) {
            parseTrackSegment();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GPXParser::parseTrackSegment() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Position &position = flight.getUserAircraft().getPosition();
    QDateTime currentDateTimeUtc;
    currentDateTimeUtc.setTimeZone(QTimeZone::utc());
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GPXParser::parseTrackSegment: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == GPX::trkpt) {
            parseTrackPoint(position, currentDateTimeUtc);
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

inline void GPXParser::parseTrackPoint(Position &position, QDateTime &currentDateTimeUtc) noexcept
{
    bool ok;
    PositionData positionData;
    const QXmlStreamAttributes attributes = d->xml.attributes();
    positionData.latitude = attributes.value(GPX::lat).toDouble(&ok);
    if (ok) {
        positionData.longitude = attributes.value(GPX::lon).toDouble(&ok);
        if (!ok) {
            d->xml.raiseError("Could not parse track longitude value.");
        }
    } else {
        d->xml.raiseError("Could not parse track latitude value.");
    }

    while (ok && d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GPXParser::parseTrackPoint: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == GPX::ele) {
            const QString elevationText = d->xml.readElementText();
            const double altitude = elevationText.toDouble(&ok);
            if (ok) {
                positionData.altitude = Convert::metersToFeet(altitude);
            } else {
                d->xml.raiseError("Could not parse track altitude value.");
            }

        } else if (d->xml.name() == GPX::time) {
            const QString dateTimeText = d->xml.readElementText();
            if (d->firstDateTimeUtc.isNull()) {
                d->firstDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
                currentDateTimeUtc = d->firstDateTimeUtc;
            } else {
                currentDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
            }
            if (currentDateTimeUtc.isValid()) {
                positionData.timestamp = d->firstDateTimeUtc.msecsTo(currentDateTimeUtc);
            } else {
                d->xml.raiseError("Invalid timestamp.");
            }
        } else {
            d->xml.skipCurrentElement();
        }
    }
    if (ok) {
        position.upsertLast(positionData);
    }
}


