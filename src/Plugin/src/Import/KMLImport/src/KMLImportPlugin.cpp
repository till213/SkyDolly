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
#include <vector>

#include <QIODevice>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QFlags>
#include <QByteArray>
#include <QList>
#include <QFileDialog>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QTimeZone>

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Model/src/SimVar.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../../Model/src/AircraftHandle.h"
#include "../../../../../Model/src/AircraftHandleData.h"
#include "../../../../../Model/src/Light.h"
#include "../../../../../Model/src/LightData.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "../../../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../../../Persistence/src/Service/FlightService.h"
#include "../../../../../Persistence/src/Service/AircraftService.h"
#include "KMLImportDialog.h"
#include "KMLImportPlugin.h"

namespace  {
    // Estimated landing speed [knots]
    constexpr double LandingVelocity = 140.0;
    // Estimated landing pitch [degrees]
    // Note: negative pitch values means "noise points upwards"
    constexpr double LandingPitch = -3.0;
    // Max banking angle [degrees]
    // https://www.pprune.org/tech-log/377244-a320-321-ap-bank-angle-limits.html
    constexpr double MaxBankAngle = 25;
}

class KMLImportPluginPrivate
{
public:
    KMLImportPluginPrivate()
        : aircraftService(std::make_unique<AircraftService>()),
          addToCurrentFlight(false),
          currentWaypointTimestamp(0)
    {}

    std::unique_ptr<AircraftService> aircraftService;
    QXmlStreamReader xml;
    Unit unit;
    AircraftType aircraftType;
    bool addToCurrentFlight;
    qint64 currentWaypointTimestamp;
    QDateTime firstDateTimeUtc;
    QDateTime currentDateTimeUtc;
    QString flightNumber;
};

// PUBLIC

KMLImportPlugin::KMLImportPlugin() noexcept
    : d(std::make_unique<KMLImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("KMLImportPlugin::KMLImportPlugin: PLUGIN LOADED");
#endif
}

KMLImportPlugin::~KMLImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("KMLImportPlugin::~KMLImportPlugin: PLUGIN UNLOADED");
#endif
}

bool KMLImportPlugin::importData(FlightService &flightService) noexcept
{
    bool ok;
    std::unique_ptr<KMLImportDialog> kmlImportDialog = std::make_unique<KMLImportDialog>(getParentWidget());
    const int choice = kmlImportDialog->exec();
    if (choice == QDialog::Accepted) {

        ok = kmlImportDialog->getSelectedAircraftType(d->aircraftType);
        if (ok) {
            d->addToCurrentFlight = kmlImportDialog->isAddToFlightEnabled();
            ok = import(kmlImportDialog->getSelectedFilePath(), flightService);
            if (ok && d->addToCurrentFlight) {
                std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
                if (skyConnect) {
                    skyConnect->get().updateAIObjects();
                }
            }
        }
    } else {
        ok = true;
    }
    return ok;
}

// PRIVATE

bool KMLImportPlugin::import(const QString &filePath, FlightService &flightService) noexcept
{
    QFile file(filePath);
    bool ok = file.open(QIODevice::ReadOnly);
    if (ok) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        if (!d->addToCurrentFlight) {
            flight.clear(true);
        }
        // The flight has at least one aircraft, but possibly without recording
        const int aircraftCount = flight.count();
        const bool addNewAircraft = d->addToCurrentFlight && (aircraftCount > 1 || flight.getUserAircraft().hasRecording());
        Aircraft &aircraft = addNewAircraft ? flight.addUserAircraft() : flight.getUserAircraft();
        d->xml.setDevice(&file);
        if (d->xml.readNextStartElement()) {
#ifdef DEBUG
            qDebug("KMLImportPlugin::import: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
            if (d->xml.name() == QLatin1String("kml")) {
                readKML();
            } else {
                d->xml.raiseError(tr("The file is not a KML file."));
            }
        }

        if (!d->xml.hasError()) {

            augmentPositionData();
            updateAircraftInfo();

            // Remember import (export) path
            const QString exportPath = QFileInfo(filePath).absolutePath();
            Settings::getInstance().setExportPath(exportPath);
            if (addNewAircraft) {
                // Sequence starts at 1
                const int newAircraftCount = flight.count();
                ok = d->aircraftService->store(flight.getId(), newAircraftCount, flight[newAircraftCount - 1]);
            } else {
                flight.setDescription(tr("Aircraft imported on %1 from file: %2").arg(d->unit.formatDateTime(QDateTime::currentDateTime()), filePath));
                flight.setCreationDate(QFileInfo(filePath).birthTime());
                updateFlightCondition();
                ok = flightService.store(flight);
            }
        } else {
#ifdef DEBUG
            qDebug("KMLImportPlugin::import: XML error: %s", qPrintable(d->xml.errorString()));
#endif
            ok = false;
        }
    }
    return ok;
}

void KMLImportPlugin::readKML() noexcept
{
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::readKML: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == QLatin1String("Document")) {
            readDocument();
        } else {
            d->xml.raiseError(tr("The file is not a KML document."));
        }
    } else {
        d->xml.raiseError(tr("Error reading the XML data."));
    }
}

void KMLImportPlugin::readDocument() noexcept
{
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::readDocument: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == QLatin1String("name")) {
            if (!d->addToCurrentFlight) {
                Flight &flight = Logbook::getInstance().getCurrentFlight();
                const QString name = d->xml.readElementText();
                flight.setTitle(name);
            }
        } else if (d->xml.name() == QLatin1String("Placemark")) {
            readPlacemark();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void KMLImportPlugin::readPlacemark() noexcept
{
    QString name;
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::readDocument: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == QLatin1String("name")) {
            name = d->xml.readElementText();
            if (name.endsWith(" Airport")) {
                // Extract the 4 letter ICAO code
                name = name.left(4);
            }
        } else if (d->xml.name() == QLatin1String("Point")) {
            readWaypoint(name);
        } else if (d->xml.name() == QLatin1String("Track")) {
            // The track contains the flight number
            d->flightNumber = name;
            readTrack();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void KMLImportPlugin::readWaypoint(const QString &icaoOrName) noexcept
{
    bool ok;
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::readWaypoint: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == QLatin1String("coordinates")) {
            const QString coordinatesText = d->xml.readElementText();
            const QStringList coordinates = coordinatesText.split(",");
            if (coordinates.count() == 3) {
                Waypoint waypoint;
                waypoint.longitude = coordinates.at(0).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(tr("Invalid longitude number."));
                }
                waypoint.latitude = coordinates.at(1).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(tr("Invalid latitude number."));
                }
                waypoint.altitude = coordinates.at(2).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(tr("Invalid altitude number."));
                }
                waypoint.identifier = icaoOrName;
                waypoint.timestamp = d->currentWaypointTimestamp;
                // The actual timestamps of the waypoints are later updated
                // with the flight duration, once the entire gx:Track data
                // has been parsed
                ++d->currentWaypointTimestamp;

                Flight &flight = Logbook::getInstance().getCurrentFlight();
                flight.getUserAircraft().getFlightPlan().add(std::move(waypoint));
            } else {
                d->xml.raiseError(tr("Invalid GPS coordinate."));
            }
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void KMLImportPlugin::readTrack() noexcept
{
    // Timestamp (msec), latitude (degrees), longitude (degrees), altitude (feet)
    typedef std::tuple<qint64, double, double, double> TrackItem;
    // The track data may contain data with identical timestamps, so we first read
    // all track data into this vector and only then "upsert" the position data
    std::vector<TrackItem> trackData;

    bool ok = true;
    int currentTrackDataIndex = 0;
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::readWaypoint: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == QLatin1String("when")) {
            const QString dateTimeText = d->xml.readElementText();
            if (d->firstDateTimeUtc.isNull()) {
                d->firstDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
                d->currentDateTimeUtc = d->firstDateTimeUtc;
            } else {
                d->currentDateTimeUtc = QDateTime::fromString(dateTimeText, Qt::ISODate);
            }
            if (d->currentDateTimeUtc.isValid()) {
                const qint64 timestamp = d->firstDateTimeUtc.msecsTo(d->currentDateTimeUtc);
                TrackItem trackItem = std::make_tuple(timestamp, 0.0, 0.0, 0.0);
                trackData.push_back(std::move(trackItem));
            } else {
                d->xml.raiseError(tr("Invalid timestamp."));
            }
        } else if (d->xml.name() == QLatin1String("coord")) {
            const QString coordinatesText = d->xml.readElementText();
            const QStringList coordinates = coordinatesText.split(" ");
            if (coordinates.count() == 3) {

                const double longitude = coordinates.at(0).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(tr("Invalid longitude number."));
                }
                const double latitude = coordinates.at(1).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(tr("Invalid latitude number."));
                }
                const double altitude = coordinates.at(2).toFloat(&ok);
                if (!ok) {
                    d->xml.raiseError(tr("Invalid altitude number."));
                }
                if (ok) {
                    std::get<1>(trackData[currentTrackDataIndex]) = latitude;
                    std::get<2>(trackData[currentTrackDataIndex]) = longitude;
                    std::get<3>(trackData[currentTrackDataIndex]) = Convert::metersToFeet(altitude);
                    ++currentTrackDataIndex;
                }

            } else {
                d->xml.raiseError(tr("Invalid GPS coordinate."));
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

        position.upsert(std::move(positionData));
    }

    // Set timezone
    d->firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    d->currentDateTimeUtc.setTimeZone(QTimeZone::utc());
}

void KMLImportPlugin::updateFlightCondition() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    FlightCondition flightCondition;

    flightCondition.startLocalTime = d->firstDateTimeUtc.toLocalTime();
    flightCondition.startZuluTime = d->firstDateTimeUtc;
    flightCondition.endLocalTime = d->currentDateTimeUtc.toLocalTime();
    flightCondition.endZuluTime = d->currentDateTimeUtc;

    flight.setFlightCondition(flightCondition);

}

void KMLImportPlugin::updateAircraftInfo() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Aircraft &aircraft = flight.getUserAircraft();
    AircraftInfo aircraftInfo(aircraft.getId());
    aircraftInfo.aircraftType = d->aircraftType;

    aircraftInfo.startDate = d->firstDateTimeUtc.toLocalTime();
    aircraftInfo.endDate = d->currentDateTimeUtc.toLocalTime();
    int positionCount = aircraft.getPosition().count();
    if (positionCount > 0) {
        const PositionData &firstPositionData = aircraft.getPosition().getFirst();
        aircraftInfo.initialAirspeed = Convert::feetPerSecondToKnots(firstPositionData.velocityBodyZ);

        int waypointCount = aircraft.getFlightPlan().count();
        if (waypointCount > 0) {
            Waypoint &departure = aircraft.getFlightPlan()[0];
            departure.altitude = firstPositionData.altitude;
            departure.localTime = d->firstDateTimeUtc.toLocalTime();
            departure.zuluTime = d->firstDateTimeUtc;

            if (waypointCount > 1) {
                const PositionData &lastPositionData = aircraft.getPosition().getLast();
                Waypoint &arrival = aircraft.getFlightPlan()[1];
                arrival.altitude = lastPositionData.altitude;
                arrival.localTime = d->currentDateTimeUtc.toLocalTime();
                arrival.zuluTime = d->currentDateTimeUtc;
            }
        }
    } else {
        aircraftInfo.initialAirspeed = 0.0;
    }
    aircraftInfo.flightNumber = d->flightNumber;
    aircraft.setAircraftInfo(aircraftInfo);
}

void KMLImportPlugin::augmentPositionData() noexcept
{
    Position &position = Logbook::getInstance().getCurrentFlight().getUserAircraft().getPosition();
    const int positionCount = position.count();
    for (int i = 0; i < positionCount; ++i) {
        if (i < positionCount - 1) {

            PositionData &startPositionData = position[i];
            const PositionData &endPositionData = position[i + 1];
            const std::pair<double, double> startPosition(startPositionData.latitude, startPositionData.longitude);
            const qint64 startTimestamp = startPositionData.timestamp;
            const std::pair<double, double> endPosition(endPositionData.latitude, endPositionData.longitude);
            const qint64 endTimestamp = endPositionData.timestamp;
            const double averageAltitude = Convert::feetToMeters((startPositionData.altitude + endPositionData.altitude) / 2.0);

            const std::pair distanceAndVelocity = SkyMath::distanceAndVelocity(startPosition, startTimestamp, endPosition, endTimestamp, averageAltitude);
            startPositionData.velocityBodyX = 0.0;
            startPositionData.velocityBodyY = 0.0;
            startPositionData.velocityBodyZ = Convert::metersPerSecondToFeetPerSecond(distanceAndVelocity.second);

            const double deltaAltitude = Convert::feetToMeters(endPositionData.altitude - startPositionData.altitude);
            // SimConnect: positive pitch values "point downward", negative pitch values "upward"
            // -> so switch the sign
            startPositionData.pitch = -SkyMath::approximatePitch(distanceAndVelocity.first, deltaAltitude);
            const double initialBearing = SkyMath::initialBearing(startPosition, endPosition);
            startPositionData.heading = initialBearing;

            if (i > 0) {
                // [-180, 180]
                const double headingChange = SkyMath::headingChange(position[i - 1].heading, startPositionData.heading);
                // We go into maximum bank angle with a heading change of 45 degrees
                // SimConnect: negative values are a "right" turn, positive values a left turn
                startPositionData.bank = qMin((std::abs(headingChange) / 45.0) * MaxBankAngle, MaxBankAngle) * SkyMath::sgn(headingChange);
            } else {
                // First point, zero bank angle
                startPositionData.bank = 0.0;
            }

        } else if (positionCount > 1){

            // Last point
            PositionData &lastPositionData = position[i];
            PositionData &previousPositionData = position[i -1];
            lastPositionData.velocityBodyX = previousPositionData.velocityBodyX;
            lastPositionData.velocityBodyY = previousPositionData.velocityBodyY;
            lastPositionData.velocityBodyZ = Convert::knotsToFeetPerSecond(LandingVelocity);

            // Attitude
            lastPositionData.pitch = LandingPitch;
            lastPositionData.bank = 0.0;
            lastPositionData.heading = previousPositionData.heading;

        } else {
            // Only one sampled data point ("academic case")
            PositionData &lastPositionData = position[i];
            lastPositionData.velocityBodyX = 0.0;
            lastPositionData.velocityBodyY = 0.0;
            lastPositionData.velocityBodyZ = 0.0;

            // Attitude
            lastPositionData.pitch = 0.0;
            lastPositionData.bank = 0.0;
            lastPositionData.heading = 0.0;
        }
    }
}
