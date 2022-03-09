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
#include "GPX.h"
#include "GPXImportSettings.h"
#include "GPXParser.h"

class GPXParserPrivate
{
public:
    GPXParserPrivate(QXmlStreamReader &xmlStreamReader, const GPXImportSettings &theImportSettings) noexcept
        : xml(xmlStreamReader),
          settings(theImportSettings),
          flight(Logbook::getInstance().getCurrentFlight()),
          position(flight.getUserAircraft().getPosition()),
          flightPlan(flight.getUserAircraft().getFlightPlan())
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    QXmlStreamReader &xml;
    const GPXImportSettings &settings;
    QDateTime firstDateTimeUtc;
    QString documentName;
    QString description;

    Flight &flight;
    Position &position;
    FlightPlan &flightPlan;
};

// PUBLIC

GPXParser::GPXParser(QXmlStreamReader &xmlStreamReader, const GPXImportSettings &theImportSettings) noexcept
    : d(std::make_unique<GPXParserPrivate>(xmlStreamReader, theImportSettings))
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

QDateTime GPXParser::getFirstDateTimeUtc() const noexcept
{
    return d->firstDateTimeUtc;
}

QString GPXParser::getDocumentName() const noexcept
{
    return d->documentName;
}

QString GPXParser::getDescription() const noexcept
{
    return d->description;
}

// PRIVATE

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
        } else if (d->xml.name() == GPX::rte) {
            parseRoute();
        } else if (d->xml.name() == GPX::trk) {
            parseTrack();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GPXParser::parseMetadata() noexcept
{
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GPXParser::parseMetadata: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == GPX::name) {
            d->documentName = d->xml.readElementText();
        } else if (d->xml.name() == GPX::desc) {
            d->description = d->xml.readElementText();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GPXParser::parseWaypoint() noexcept
{
    bool ok {true};
    double latitude, longitude, altitude;
    QString identifier;
    QDateTime currentDateTimeUtc;

    if (d->settings.m_waypointSelection == GPXImportSettings::GPXElement::Waypoint ||
        d->settings.m_positionSelection == GPXImportSettings::GPXElement::Waypoint) {
        ok = parseWaypointType(latitude, longitude, altitude, identifier, currentDateTimeUtc);
    } else {
        d->xml.skipCurrentElement();
    }

    if (ok && d->settings.m_waypointSelection == GPXImportSettings::GPXElement::Waypoint) {
        Waypoint waypoint {static_cast<float>(latitude), static_cast<float>(longitude), static_cast<float>(altitude)};
        waypoint.identifier = identifier;
        d->flightPlan.add(waypoint);
    }
    if (ok && d->settings.m_positionSelection == GPXImportSettings::GPXElement::Waypoint) {
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
            const double velocityMetersPerSecond = Convert::knotsToMetersPerSecond(d->settings.m_defaultVelocity);
            const double seconds = distance / velocityMetersPerSecond;
            // Milliseconds
            positionData.timestamp = previousPositionData.timestamp + qRound(seconds * 1000.0);
        } else {
            positionData.timestamp = 0;
        }

        d->position.upsertLast(positionData);
    }
}

void GPXParser::parseRoute() noexcept
{    
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GPXParser::parseRoute: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == GPX::name) {
            // The route name takes precedence over the name given in the metadata
            d->documentName = d->xml.readElementText();
        } else if (d->xml.name() == GPX::desc) {
            // The route description takes precedence over the description given in the metadata
            d->description = d->xml.readElementText();
        } else if (d->xml.name() == GPX::rtept) {
            parseRoutePoint();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void GPXParser::parseRoutePoint() noexcept
{
    bool ok {true};
    double latitude, longitude, altitude;
    QString identifier;
    QDateTime currentDateTimeUtc;

    if (d->settings.m_waypointSelection == GPXImportSettings::GPXElement::Route ||
        d->settings.m_positionSelection == GPXImportSettings::GPXElement::Route) {
        ok = parseWaypointType(latitude, longitude, altitude, identifier, currentDateTimeUtc);
    } else {
        d->xml.skipCurrentElement();
    }

    if (ok && d->settings.m_waypointSelection == GPXImportSettings::GPXElement::Route) {
        Waypoint waypoint {static_cast<float>(latitude), static_cast<float>(longitude), static_cast<float>(altitude)};
        waypoint.identifier = identifier;
        d->flightPlan.add(waypoint);
    }
    if (ok && d->settings.m_positionSelection == GPXImportSettings::GPXElement::Route) {
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
            const double velocityMetersPerSecond = Convert::knotsToMetersPerSecond(d->settings.m_defaultVelocity);
            const double seconds = distance / velocityMetersPerSecond;
            // Milliseconds
            positionData.timestamp = previousPositionData.timestamp + qRound(seconds * 1000.0);
        } else {
            positionData.timestamp = 0;
        }

        d->position.upsertLast(positionData);
    }
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
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("GPXParser::parseTrackSegment: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == GPX::trkpt) {
            parseTrackPoint();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

inline void GPXParser::parseTrackPoint() noexcept
{
    bool ok {true};
    double latitude, longitude, altitude;
    QString identifier;
    QDateTime currentDateTimeUtc;

    if (d->settings.m_waypointSelection == GPXImportSettings::GPXElement::Track ||
        d->settings.m_positionSelection == GPXImportSettings::GPXElement::Track) {
        ok = parseWaypointType(latitude, longitude, altitude, identifier, currentDateTimeUtc);
    } else {
        d->xml.skipCurrentElement();
    }

    if (ok && d->settings.m_waypointSelection == GPXImportSettings::GPXElement::Track) {
        Waypoint waypoint {static_cast<float>(latitude), static_cast<float>(longitude), static_cast<float>(altitude)};
        waypoint.identifier = identifier;
        d->flightPlan.add(waypoint);
    }
    if (ok && d->settings.m_positionSelection == GPXImportSettings::GPXElement::Track) {
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
                const double velocityMetersPerSecond = Convert::knotsToMetersPerSecond(d->settings.m_defaultVelocity);
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

bool GPXParser::parseWaypointType(double &latitude, double &longitude, double &altitude, QString &identifier, QDateTime &dateTime) noexcept
{
    bool ok;

    const QXmlStreamAttributes attributes = d->xml.attributes();
    latitude = attributes.value(GPX::lat).toDouble(&ok);
    if (ok) {
        longitude = attributes.value(GPX::lon).toDouble(&ok);
        if (!ok) {
            d->xml.raiseError("Could not parse waypoint longitude value.");
        }
    } else {
        d->xml.raiseError("Could not parse waypoint latitude value.");
    }

    altitude = d->settings.m_defaultAltitude;
    while (ok && d->xml.readNextStartElement()) {
        if (d->xml.name() == GPX::ele) {
            const QString elevationText = d->xml.readElementText();
            altitude = Convert::metersToFeet(elevationText.toDouble(&ok));
            if (!ok) {
                d->xml.raiseError("Could not parse waypoint altitude value.");
            }
        } else if (d->xml.name() == GPX::name) {
            identifier = d->xml.readElementText();
        } else if (d->xml.name() == GPX::time) {
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
