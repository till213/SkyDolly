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
#include <QStringLiteral>
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
#include "../../../../../Flight/src/FlightAugmentation.h"
#include "../../../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../../../Persistence/src/Service/FlightService.h"
#include "../../../../../Persistence/src/Service/AircraftService.h"
#include "KMLImportDialog.h"
#include "KMLImportSettings.h"
#include "KMLImportPlugin.h"

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
    KMLImportSettings importSettings;
    AircraftType aircraftType;
    bool addToCurrentFlight;
    qint64 currentWaypointTimestamp;
    QDateTime firstDateTimeUtc;
    QDateTime currentDateTimeUtc;
    QString flightNumber;
    FlightAugmentation flightAugmentation;
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
    std::unique_ptr<KMLImportDialog> importDialog = std::make_unique<KMLImportDialog>(d->importSettings, getParentWidget());
    const int choice = importDialog->exec();
    if (choice == QDialog::Accepted) {
        // Remember import (export) path
        const QString filePath = QFileInfo(importDialog->getSelectedFilePath()).absolutePath();
        Settings::getInstance().setExportPath(filePath);
        ok = importDialog->getSelectedAircraftType(d->aircraftType);
        if (ok) {
            d->addToCurrentFlight = importDialog->isAddToFlightEnabled();
            ok = import(importDialog->getSelectedFilePath(), flightService);
            if (ok) {
                if (d->addToCurrentFlight) {
                    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
                    if (skyConnect) {
                        skyConnect->get().updateAIObjects();
                    }
                }
            } else {
                QMessageBox::critical(getParentWidget(), tr("Import error"), tr("The KML file %1 could not be imported.").arg(filePath));
            }
        }
    } else {
        ok = true;
    }
    return ok;
}

// PROTECTED

Settings::PluginSettings KMLImportPlugin::getSettings() const noexcept
{
    return d->importSettings.getSettings();
}

Settings::KeysWithDefaults KMLImportPlugin::getKeyWithDefaults() const noexcept
{
    return d->importSettings.getKeysWithDefault();
}

void KMLImportPlugin::setSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    d->importSettings.setSettings(valuesByKey);
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
            if (d->xml.name() == QStringLiteral("kml")) {
                readKML();
            } else {
                d->xml.raiseError(tr("The file is not a KML file."));
            }
        }

        // Also ensure that at least one position could be imported;
        // depending on the "KML flavour" that might not always succeed,
        // keyword: placemarks vs line segments etc.
        if (!d->xml.hasError() && aircraft.getPositionConst().count() > 0) {
            d->flightAugmentation.augmentAircraftData(aircraft);
            updateAircraftInfo();
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
            qDebug("KMLImportPlugin::import: XML error: %s - number of imported positions: %lu", qPrintable(d->xml.errorString()), aircraft.getPositionConst().count());
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
        if (d->xml.name() == QStringLiteral("Document")) {
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
        if (d->xml.name() == QStringLiteral("name")) {
            if (!d->addToCurrentFlight) {
                Flight &flight = Logbook::getInstance().getCurrentFlight();
                const QString name = d->xml.readElementText();
                flight.setTitle(name);
            } else {
                // Flight keeps its existing title (name)
                d->xml.skipCurrentElement();
            }
        } else if (d->xml.name() == QStringLiteral("Placemark")) {
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
        if (d->xml.name() == QStringLiteral("name")) {
            name = d->xml.readElementText();
            if (name.endsWith(" Airport")) {
                // Extract the 4 letter ICAO code
                name = name.left(4);
            }
        } else if (d->xml.name() == QStringLiteral("Point")) {
            readWaypoint(name);
        } else if (d->xml.name() == QStringLiteral("Track")) {
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
        if (d->xml.name() == QStringLiteral("coordinates")) {
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
        if (d->xml.name() == QStringLiteral("when")) {
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
        } else if (d->xml.name() == QStringLiteral("coord")) {
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

        position.upsertLast(std::move(positionData));
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
