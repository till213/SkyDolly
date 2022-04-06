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
#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "Gpx.h"
#include "GpxImportSettings.h"
#include "GpxParser.h"

class GpxParserPrivate
{
public:
    GpxParserPrivate(QXmlStreamReader &xmlStreamReader, const GpxImportSettings &theImportSettings) noexcept
        : xml(xmlStreamReader),
          settings(theImportSettings),
          flight(Logbook::getInstance().getCurrentFlight()),
          position(flight.getUserAircraft().getPosition()),
          flightPlan(flight.getUserAircraft().getFlightPlan())
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    QXmlStreamReader &xml;
    const GpxImportSettings &settings;
    QDateTime firstDateTimeUtc;
    QString documentName;
    QString description;

    Flight &flight;
    // @todo Store a list of Positions: whenever a new <track> is encountered:
    //      - if the timestamp is greater than the last timestamp -> continue the existing track (keep adding to current Positions)
    //      - else start a new track (add a new Position list entry)
    Position &position;
    FlightPlan &flightPlan;
};

// PUBLIC

GpxParser::GpxParser(QXmlStreamReader &xmlStreamReader, const GpxImportSettings &theImportSettings) noexcept
    : d(std::make_unique<GpxParserPrivate>(xmlStreamReader, theImportSettings))
{
#ifdef DEBUG
    qDebug("GpxParser::~GpxParser: CREATED");
#endif
}

GpxParser::~GpxParser() noexcept
{
#ifdef DEBUG
    qDebug("GpxParser::~GpxParser: DELETED");
#endif
}

void GpxParser::parse() noexcept
{
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GpxParser::parse: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == Gpx::gpx) {
            parseGPX();
        } else {
            d->xml.raiseError(QStringLiteral("The file is not a GPX file."));
        }
    }
}

QDateTime GpxParser::getFirstDateTimeUtc() const noexcept
{
    return d->firstDateTimeUtc;
}

QString GpxParser::getDocumentName() const noexcept
{
    return d->documentName;
}

QString GpxParser::getDescription() const noexcept
{
    return d->description;
}

// PRIVATE

void GpxParser::parseGPX() noexcept
{
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GpxParser::parseGPX: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == Gpx::metadata) {
            parseMetadata();
        } else if (d->xml.name() == Gpx::wpt) {
            parseWaypoint();
        } else if (d->xml.name() == Gpx::rte) {
            parseRoute();
        } else if (d->xml.name() == Gpx::trk) {
            parseTrack();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GpxParser::parseMetadata() noexcept
{
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GpxParser::parseMetadata: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == Gpx::name) {
            d->documentName = d->xml.readElementText();
        } else if (d->xml.name() == Gpx::desc) {
            d->description = d->xml.readElementText();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GpxParser::parseWaypoint() noexcept
{
    bool ok {true};
    double latitude, longitude, altitude;
    QString identifier;
    QDateTime currentDateTimeUtc;

    if (d->settings.getWaypointSelection() == GpxImportSettings::GPXElement::Waypoint ||
        d->settings.getPositionSelection() == GpxImportSettings::GPXElement::Waypoint) {
        ok = parseWaypointType(latitude, longitude, altitude, identifier, currentDateTimeUtc);
    } else {
        d->xml.skipCurrentElement();
    }

    if (ok && d->settings.getWaypointSelection() == GpxImportSettings::GPXElement::Waypoint) {
        Waypoint waypoint {static_cast<float>(latitude), static_cast<float>(longitude), static_cast<float>(altitude)};
        waypoint.identifier = identifier;
        d->flightPlan.add(waypoint);
    }
    if (ok && d->settings.getPositionSelection() == GpxImportSettings::GPXElement::Waypoint) {
        PositionData positionData {latitude, longitude, altitude};
        if (d->firstDateTimeUtc.isNull()) {
            if (currentDateTimeUtc.isValid()) {
                d->firstDateTimeUtc = currentDateTimeUtc;
            } else {
                d->firstDateTimeUtc = QDateTime::currentDateTimeUtc();
            }
        }

        // We ignore waypoint timestamps for positions; always calculated based on default velocity and distance
        if (d->position.count() > 0) {
            const PositionData &previousPositionData = d->position.getLast();
            const SkyMath::Coordinate start = {previousPositionData.latitude, previousPositionData.longitude};
            const SkyMath::Coordinate end = {positionData.latitude, positionData.longitude};
            const double averageAltitude = (previousPositionData.altitude + positionData.altitude) / 2.0;
            // In meters
            const double distance = SkyMath::sphericalDistance(start, end, averageAltitude);
            const double velocityMetersPerSecond = Convert::knotsToMetersPerSecond(d->settings.getDefaultVelocity());
            const double seconds = distance / velocityMetersPerSecond;
            // Milliseconds
            positionData.timestamp = previousPositionData.timestamp + qRound(seconds * 1000.0);
        } else {
            positionData.timestamp = 0;
        }

        d->position.upsertLast(positionData);
    }
}

void GpxParser::parseRoute() noexcept
{    
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GpxParser::parseRoute: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == Gpx::name) {
            // The route name takes precedence over the name given in the metadata
            d->documentName = d->xml.readElementText();
        } else if (d->xml.name() == Gpx::desc) {
            // The route description takes precedence over the description given in the metadata
            d->description = d->xml.readElementText();
        } else if (d->xml.name() == Gpx::rtept) {
            parseRoutePoint();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GpxParser::parseRoutePoint() noexcept
{
    bool ok {true};
    double latitude, longitude, altitude;
    QString identifier;
    QDateTime currentDateTimeUtc;

    if (d->settings.getWaypointSelection() == GpxImportSettings::GPXElement::Route ||
        d->settings.getPositionSelection() == GpxImportSettings::GPXElement::Route) {
        ok = parseWaypointType(latitude, longitude, altitude, identifier, currentDateTimeUtc);
    } else {
        d->xml.skipCurrentElement();
    }

    if (ok && d->settings.getWaypointSelection() == GpxImportSettings::GPXElement::Route) {
        Waypoint waypoint {static_cast<float>(latitude), static_cast<float>(longitude), static_cast<float>(altitude)};
        waypoint.identifier = identifier;
        d->flightPlan.add(waypoint);
    }
    if (ok && d->settings.getPositionSelection() == GpxImportSettings::GPXElement::Route) {
        PositionData positionData {latitude, longitude, altitude};
        if (d->firstDateTimeUtc.isNull()) {
            if (currentDateTimeUtc.isValid()) {
                d->firstDateTimeUtc = currentDateTimeUtc;
            } else {
                d->firstDateTimeUtc = QDateTime::currentDateTimeUtc();
            }
        }

        // We ignore route point timestamps for positions; always calculated based on default velocity and distance
        if (d->position.count() > 0) {
            const PositionData &previousPositionData = d->position.getLast();
            const SkyMath::Coordinate start = {previousPositionData.latitude, previousPositionData.longitude};
            const SkyMath::Coordinate end = {positionData.latitude, positionData.longitude};
            const double averageAltitude = (previousPositionData.altitude + positionData.altitude) / 2.0;
            // In meters
            const double distance = SkyMath::sphericalDistance(start, end, averageAltitude);
            const double velocityMetersPerSecond = Convert::knotsToMetersPerSecond(d->settings.getDefaultVelocity());
            const double seconds = distance / velocityMetersPerSecond;
            // Milliseconds
            positionData.timestamp = previousPositionData.timestamp + qRound(seconds * 1000.0);
        } else {
            positionData.timestamp = 0;
        }

        d->position.upsertLast(positionData);
    }
}

void GpxParser::parseTrack() noexcept
{
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GpxParser::parseTrack: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == Gpx::trkseg) {
            parseTrackSegment();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GpxParser::parseTrackSegment() noexcept
{
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GpxParser::parseTrackSegment: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == Gpx::trkpt) {
            parseTrackPoint();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

inline void GpxParser::parseTrackPoint() noexcept
{
    bool ok {true};
    double latitude, longitude, altitude;
    QString identifier;
    QDateTime currentDateTimeUtc;

    if (d->settings.getWaypointSelection() == GpxImportSettings::GPXElement::Track ||
        d->settings.getPositionSelection() == GpxImportSettings::GPXElement::Track) {
        ok = parseWaypointType(latitude, longitude, altitude, identifier, currentDateTimeUtc);
    } else {
        d->xml.skipCurrentElement();
    }

    if (ok && d->settings.getWaypointSelection() == GpxImportSettings::GPXElement::Track) {
        Waypoint waypoint {static_cast<float>(latitude), static_cast<float>(longitude), static_cast<float>(altitude)};
        waypoint.identifier = identifier;
        d->flightPlan.add(waypoint);
    }
    if (ok && d->settings.getPositionSelection() == GpxImportSettings::GPXElement::Track) {
        PositionData positionData {latitude, longitude, altitude};
        if (d->firstDateTimeUtc.isNull()) {
            if (currentDateTimeUtc.isValid()) {
                d->firstDateTimeUtc = currentDateTimeUtc;
            } else {
                d->firstDateTimeUtc = QDateTime::currentDateTimeUtc();
            }
        }

        if (currentDateTimeUtc.isValid()) {
            positionData.timestamp = d->firstDateTimeUtc.msecsTo(currentDateTimeUtc);
        } else {
            // No timestamp available, so calculate timestamp based on default velocity and distance
            if (d->position.count() > 0) {
                const PositionData &previousPositionData = d->position.getLast();
                const SkyMath::Coordinate start = {previousPositionData.latitude, previousPositionData.longitude};
                const SkyMath::Coordinate end = {positionData.latitude, positionData.longitude};
                const double averageAltitude = (previousPositionData.altitude + positionData.altitude) / 2.0;
                // In meters
                const double distance = SkyMath::sphericalDistance(start, end, averageAltitude);
                const double velocityMetersPerSecond = Convert::knotsToMetersPerSecond(d->settings.getDefaultVelocity());
                const double seconds = distance / velocityMetersPerSecond;
                // Milliseconds
                positionData.timestamp = previousPositionData.timestamp + qRound(seconds * 1000.0);
            } else {
                positionData.timestamp = 0;
            }
        }

        d->position.upsertLast(positionData);
    }
}

bool GpxParser::parseWaypointType(double &latitude, double &longitude, double &altitude, QString &identifier, QDateTime &dateTime) noexcept
{
    bool ok;

    const QXmlStreamAttributes attributes = d->xml.attributes();
    latitude = attributes.value(Gpx::lat).toDouble(&ok);
    if (ok) {
        longitude = attributes.value(Gpx::lon).toDouble(&ok);
        if (!ok) {
            d->xml.raiseError("Could not parse waypoint longitude value.");
        }
    } else {
        d->xml.raiseError("Could not parse waypoint latitude value.");
    }

    altitude = d->settings.getDefaultAltitude();
    while (ok && d->xml.readNextStartElement()) {
        if (d->xml.name() == Gpx::ele) {
            const QString elevationText = d->xml.readElementText();
            altitude = Convert::metersToFeet(elevationText.toDouble(&ok));
            if (!ok) {
                d->xml.raiseError("Could not parse waypoint altitude value.");
            }
        } else if (d->xml.name() == Gpx::name) {
            identifier = d->xml.readElementText();
        } else if (d->xml.name() == Gpx::time) {
            dateTime = QDateTime::fromString(d->xml.readElementText(), Qt::ISODate);
            ok = dateTime.isValid();
            if (!ok) {
                d->xml.raiseError("Invalid timestamp.");
            }
        } else {
            d->xml.skipCurrentElement();
        }
    }

    return ok;
}
