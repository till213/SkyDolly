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
#include <tuple>
#include <cstdint>
#include <exception>

#include <QString>
#include <QStringLiteral>
#include <QTimeZone>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QCoreApplication>

#include <GeographicLib/Geoid.hpp>

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Waypoint.h"
#include "Kml.h"
#include "AbstractKmlParser.h"
#include "AbstractKmlTrackParser.h"

class AbstractKmlTrackParserPrivate
{
public:
    AbstractKmlTrackParserPrivate() noexcept
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    GeographicLib::Geoid egm96 {"egm2008-5", QCoreApplication::applicationDirPath().append("/geoids").toStdString()};
    QDateTime firstDateTimeUtc;
};

// PUBLIC

AbstractKmlTrackParser::AbstractKmlTrackParser() noexcept
    : AbstractKmlParser(),
      d(std::make_unique<AbstractKmlTrackParserPrivate>())
{
#ifdef DEBUG
    qDebug("AbstractKmlTrackParser::AbstractKmlTrackParser: CREATED");
#endif
}

AbstractKmlTrackParser::~AbstractKmlTrackParser() noexcept
{
#ifdef DEBUG
    qDebug("AbstractKmlTrackParser::~AbstractKmlTrackParser: DELETED");
#endif
}

QDateTime AbstractKmlTrackParser::getFirstDateTimeUtc() const noexcept
{
    return d->firstDateTimeUtc;
}

// PROTECTED

void AbstractKmlTrackParser::parseTrack() noexcept
{
    const Flight *flight = getFlight();
    QXmlStreamReader *xml = getXmlStreamReader();
    Position &position = flight->getUserAircraft().getPosition();
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
        while (xml->readNextStartElement()) {
            const QStringRef xmlName = xml->name();
#ifdef DEBUG
        qDebug("AbstractKmlTrackParser::parseTrack: XML start element: %s", qPrintable(xmlName.toString()));
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
                    const std::int64_t timestamp = d->firstDateTimeUtc.msecsTo(currentDateTimeUtc);
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

                        try {

                          // Convert height above egm96 to height above the ellipsoid

                          double
                            geoid_height = d->egm96(latitude, longitude),
                            height_above_geoid = (altitude +
                                                      GeographicLib::Geoid::ELLIPSOIDTOGEOID * geoid_height);
                          qDebug("height_above_geoid: %f", height_above_geoid);
                        }
                        catch (const std::exception& e) {
                          qDebug("Caught exception: %s", e.what());

                        }

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
        for (const TrackItem &trackItem : trackData) {
            PositionData positionData {std::get<1>(trackItem), std::get<2>(trackItem), std::get<3>(trackItem)};
            positionData.timestamp = std::get<0>(trackItem);

            position.upsertLast(std::move(positionData));
        }

    } else {
        // We have already encountered track data, so skip all subsequent ones
        // (assuming that the relevant position data is in the first track of
        // the KML document)
        xml->skipCurrentElement();
    }

}
