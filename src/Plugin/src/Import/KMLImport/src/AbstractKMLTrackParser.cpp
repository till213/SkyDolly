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
#include "AbstractKMLTrackParser.h"

class AbstractKMLTrackParserPrivate
{
public:
    AbstractKMLTrackParserPrivate(QXmlStreamReader &xmlStreamReader) noexcept
        : xml(xmlStreamReader)
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    QXmlStreamReader &xml;
    QDateTime firstDateTimeUtc;
};

// PUBLIC

AbstractKMLTrackParser::AbstractKMLTrackParser(QXmlStreamReader &xmlStreamReader) noexcept
    : AbstractKMLParser(xmlStreamReader),
      d(std::make_unique<AbstractKMLTrackParserPrivate>(xmlStreamReader))
{
#ifdef DEBUG
    qDebug("AbstractKMLTrackParser::~AbstractKMLTrackParser: CREATED");
#endif
}

AbstractKMLTrackParser::~AbstractKMLTrackParser() noexcept
{
#ifdef DEBUG
    qDebug("AbstractKMLTrackParser::~AbstractKMLTrackParser: DELETED");
#endif
}

QDateTime AbstractKMLTrackParser::getFirstDateTimeUtc() const noexcept
{
    return d->firstDateTimeUtc;
}

// PROTECTED

void AbstractKMLTrackParser::parseTrack() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Position &position = flight.getUserAircraft().getPosition();
    if (position.count() == 0) {

        // Timestamp (msec), latitude (degrees), longitude (degrees), altitude (feet)
        typedef std::tuple<std::int64_t, double, double, double> TrackItem;
        // The track data - <when> and <gx:coord> - may be interleaved or "parallel" (first
        // all <when> timestamps, then all <coord>). So we first read all timestamped
        // coordinates into the trackData vector, and only then update the flight positions,
        // also taking care of possible duplicate timestamps (-> "upsert")
        std::vector<TrackItem> trackData;
        QDateTime currentDateTimeUtc;
        currentDateTimeUtc.setTimeZone(QTimeZone::utc());

        bool ok = true;
        int currentTrackDataIndex = 0;
        while (d->xml.readNextStartElement()) {
            const QStringRef xmlName = d->xml.name();
#ifdef DEBUG
        qDebug("AbstractKMLTrackParser::parseTrack: XML start element: %s", qPrintable(xmlName.toString()));
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
                    d->xml.raiseError("Invalid timestamp.");
                }
            } else if (xmlName == KML::coord) {
                const QString coordinatesText = d->xml.readElementText();
                const QStringList coordinates = coordinatesText.split(" ");
                if (coordinates.count() == 3) {

                    const double longitude = coordinates.at(0).toDouble(&ok);
                    if (!ok) {
                        d->xml.raiseError("Invalid longitude number.");
                    }
                    const double latitude = coordinates.at(1).toDouble(&ok);
                    if (!ok) {
                        d->xml.raiseError("Invalid latitude number.");
                    }
                    const double altitude = coordinates.at(2).toDouble(&ok);
                    if (!ok) {
                        d->xml.raiseError("Invalid altitude number.");
                    }
                    if (ok) {
                        std::get<1>(trackData[currentTrackDataIndex]) = latitude;
                        std::get<2>(trackData[currentTrackDataIndex]) = longitude;
                        std::get<3>(trackData[currentTrackDataIndex]) = Convert::metersToFeet(altitude);
                        ++currentTrackDataIndex;
                    }

                } else {
                    d->xml.raiseError("Invalid GPS coordinate.");
                }
            } else {
                d->xml.skipCurrentElement();
            }
        }

        // Now "upsert" the position data, taking possible duplicate timestamps into account
        for (const TrackItem &trackItem : trackData) {
            PositionData positionData;
            positionData.timestamp = std::get<0>(trackItem);
            positionData.latitude = std::get<1>(trackItem);
            positionData.longitude = std::get<2>(trackItem);
            positionData.altitude = std::get<3>(trackItem);

            position.upsertLast(std::move(positionData));
        }

    } else {
        // We have already encountered track data, so skip all subsequent ones
        // (assuming that the relevant position data is in the first track of
        // the KML document)
        d->xml.skipCurrentElement();
    }

}
