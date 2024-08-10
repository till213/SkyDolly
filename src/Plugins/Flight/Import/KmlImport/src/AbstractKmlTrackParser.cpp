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
#include <tuple>
#include <cstdint>

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
#include "AbstractKmlTrackParser.h"

struct AbstractKmlTrackParserPrivate
{
    AbstractKmlTrackParserPrivate() noexcept
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::UTC);
    }

    QDateTime firstDateTimeUtc;
};

// PUBLIC

AbstractKmlTrackParser::AbstractKmlTrackParser() noexcept
    : AbstractKmlParser(),
      d {std::make_unique<AbstractKmlTrackParserPrivate>()}
{}

AbstractKmlTrackParser::~AbstractKmlTrackParser() = default;

// PROTECTED

void AbstractKmlTrackParser::parseTrack(FlightData &flightData) noexcept
{
    auto *xml = getXmlStreamReader();
    Position &position = flightData.getUserAircraft().getPosition();
    if (position.count() == 0) {

        // Timestamp (msec), latitude (degrees), longitude (degrees), altitude (feet)
        using TrackItem = std::tuple<std::int64_t, double, double, double>;
        // The track data - <when> and <gx:coord> - may be interleaved or "parallel" (first
        // all <when> timestamps, then all <coord>). So we first read all timestamped
        // coordinates into the trackData vector, and only then update the flight positions,
        // also taking care of possible duplicate timestamps (-> "upsert")
        std::vector<TrackItem> trackData;
        QDateTime currentDateTimeUtc;
        currentDateTimeUtc.setTimeZone(QTimeZone::UTC);

        bool ok {true};
        int currentTrackDataIndex = 0;
        while (xml->readNextStartElement()) {
            const QStringView xmlName = xml->name();
#ifdef DEBUG
            qDebug() << "AbstractKmlTrackParser::parseTrack: XML start element:" << xmlName.toString();
#endif
            if (xmlName == Kml::when) {
                const QString dateTimeText = xml->readElementText();
                if (d->firstDateTimeUtc.isNull()) {
                    d->firstDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
                    currentDateTimeUtc = d->firstDateTimeUtc;
                } else {
                    currentDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
                }
                if (currentDateTimeUtc.isValid()) {
                    const auto timestamp = d->firstDateTimeUtc.msecsTo(currentDateTimeUtc);
                    TrackItem trackItem = std::make_tuple(timestamp, 0.0, 0.0, 0.0);
                    trackData.push_back(std::move(trackItem));
                } else {
                    xml->raiseError("Invalid timestamp.");
                }
            } else if (xmlName == Kml::coord) {
                const QString coordinatesText = xml->readElementText();
                const QStringList coordinates = coordinatesText.split(" ");
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
                        std::get<1>(trackData[currentTrackDataIndex]) = latitude;
                        std::get<2>(trackData[currentTrackDataIndex]) = longitude;
                        std::get<3>(trackData[currentTrackDataIndex]) = Convert::metersToFeet(altitude);
                        ++currentTrackDataIndex;
                    }

                } else {
                    xml->raiseError("Invalid GPS coordinate.");
                }
            } else {
                xml->skipCurrentElement();
            }
        }

        // Now "upsert" the position data, taking possible duplicate timestamps into account
        for (const auto &trackItem : trackData) {
            PositionData positionData {std::get<1>(trackItem), std::get<2>(trackItem), std::get<3>(trackItem)};
            positionData.timestamp = std::get<0>(trackItem);

            position.upsertLast(positionData);
        }

    } else {
        // We have already encountered track data, so skip all subsequent ones
        // (assuming that the relevant position data is in the first track of
        // the KML document)
        xml->skipCurrentElement();
    }
}

QDateTime AbstractKmlTrackParser::getFirstDateTimeUtc() const
{
    return d->firstDateTimeUtc;
}
