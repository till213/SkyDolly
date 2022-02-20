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
#include <QRegularExpression>

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Waypoint.h"
#include "KML.h"
#include "GenericKMLParser.h"

class GenericKMLParserPrivate
{
public:
    typedef struct {
        std::int64_t timestamp;
        double latitude;
        double longitude;
        double altitude;
    } TrackItem;
    // The track data may contain data with identical timestamps
    std::vector<TrackItem> trackData;

    GenericKMLParserPrivate(QXmlStreamReader &xmlStreamReader) noexcept
        : xml(xmlStreamReader),
          currentWaypointTimestamp(0)
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    QXmlStreamReader &xml;
    QString name;
    QString flightNumber;
    std::int64_t currentWaypointTimestamp;
    QDateTime firstDateTimeUtc;
};

// PUBLIC

GenericKMLParser::GenericKMLParser(QXmlStreamReader &xmlStreamReader) noexcept
    : d(std::make_unique<GenericKMLParserPrivate>(xmlStreamReader))
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
void GenericKMLParser::parse(QDateTime &firstDateTimeUtc, QString &name, QString &flightNumber) noexcept
{
    d->trackData.clear();

    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
        if (xmlName == KML::Placemark) {
            parsePlacemark();
        } else if (xmlName == KML::Document) {
            parseDocument();
        } else if (xmlName == KML::Folder) {
            parseFolder();
        } else {
            qDebug("Skippping element: %s", qPrintable(xmlName.toString()));
            d->xml.skipCurrentElement();
        }
    }

    // Now "upsert" the position data, taking duplicate timestamps into account
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Position &position = flight.getUserAircraft().getPosition();
    for (const GenericKMLParserPrivate::TrackItem &trackItem : d->trackData) {
        PositionData positionData;
        positionData.timestamp = trackItem.timestamp;
        positionData.latitude = trackItem.latitude;
        positionData.longitude = trackItem.longitude;
        positionData.altitude = trackItem.altitude;
        position.upsertLast(std::move(positionData));
    }

    firstDateTimeUtc = d->firstDateTimeUtc;
    name = d->name;
    flightNumber = d->flightNumber;
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

void GenericKMLParser::parseTrack() noexcept
{
    // Timestamp (msec), latitude (degrees), longitude (degrees), altitude (feet)
    typedef std::tuple<std::int64_t, double, double, double> TrackItem;
    // The track data may contain data with identical timestamps, so we first read
    // all track data into this vector and only then "upsert" the position data
    std::vector<TrackItem> trackData;
    QDateTime currentDateTimeUtc;
    currentDateTimeUtc.setTimeZone(QTimeZone::utc());

    bool ok = true;
    int currentTrackDataIndex = 0;
    while (d->xml.readNextStartElement()) {
        const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("GenericKMLParser::parseTrack: XML start element: %s", qPrintable(xmlName.toString()));
#endif
        if (xmlName == KML::when) {
            const QString dateTimeText = d->xml.readElementText();
            if (d->firstDateTimeUtc.isNull()) {
                d->firstDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
                currentDateTimeUtc = d->firstDateTimeUtc;
            } else {
                currentDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
            }
            if (currentDateTimeUtc.isValid()) {
                const std::int64_t timestamp = d->firstDateTimeUtc.msecsTo(currentDateTimeUtc);
                TrackItem trackItem = std::make_tuple(timestamp, 0.0, 0.0, 0.0);
                trackData.push_back(std::move(trackItem));
            } else {
                d->xml.raiseError(QStringLiteral("Invalid timestamp."));
            }
        } else if (xmlName == KML::coord) {
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
}
