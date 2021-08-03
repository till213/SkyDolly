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
#include "../../../../../Model/src/SimVar.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
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

class KMLImportPluginPrivate
{
public:
    KMLImportPluginPrivate()
        : aircraftService(std::make_unique<AircraftService>()),
          addToCurrentFlight(false),
          currentWaypointTimestamp(0),
          currentPositionIndex(0)
    {}

    std::unique_ptr<AircraftService> aircraftService;
    QXmlStreamReader xml;
    Unit unit;
    bool addToCurrentFlight;
    qint64 currentWaypointTimestamp;
    int currentPositionIndex;
    QDateTime firstDateTime;
    QDateTime currentDateTime;
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
        AircraftType aircraftType;
        ok = kmlImportDialog->getSelectedAircraftType(aircraftType);
        d->addToCurrentFlight = kmlImportDialog->isAddToFlightEnabled();
        if (ok) {
            ok = import(kmlImportDialog->getSelectedFilePath(), aircraftType, flightService);
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

bool KMLImportPlugin::import(const QString &filePath, const AircraftType &aircraftType, FlightService &flightService) noexcept
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

            // Remember import (export) path
            const QString exportPath = QFileInfo(filePath).absolutePath();
            Settings::getInstance().setExportPath(exportPath);

            AircraftInfo info(aircraft.getId());
            info.aircraftType = aircraftType;
            info.startDate = QFileInfo(filePath).birthTime();
            info.endDate = info.startDate.addMSecs(aircraft.getDurationMSec());
            aircraft.setAircraftInfo(info);
            if (addNewAircraft) {
                // Sequence starts at 1
                const int newAircraftCount = flight.count();
                ok = d->aircraftService->store(flight.getId(), newAircraftCount, flight[newAircraftCount - 1]);
            } else {
                flight.setDescription(tr("Aircraft imported on %1 from file: %2").arg(d->unit.formatDateTime(QDateTime::currentDateTime()), filePath));
                ok = flightService.store(flight);
            }
        } else {

            qDebug("XML Error: %s", qPrintable(d->xml.errorString()));
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
        } else if (d->xml.name() == QLatin1String("Point")) {
            readWaypoint(name);
        } else if (d->xml.name() == QLatin1String("Track")) {
            readTrack();
        } else {
            d->xml.skipCurrentElement();
        }
    }
}

void KMLImportPlugin::readWaypoint(const QString &name) noexcept
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
                waypoint.identifier = name;
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
    bool ok;
    while (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::readWaypoint: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == QLatin1String("when")) {
            const QString dateTimeText = d->xml.readElementText();
            if (d->firstDateTime.isNull()) {
                d->firstDateTime = QDateTime::fromString(dateTimeText, Qt::ISODate);
                d->firstDateTime.setTimeZone(QTimeZone::utc());
                d->currentDateTime = d->firstDateTime;
            } else {
                d->currentDateTime = QDateTime::fromString(dateTimeText, Qt::ISODate);
                d->currentDateTime.setTimeZone(QTimeZone::utc());
            }
            if (d->currentDateTime.isValid()) {
                const qint64 timestamp = d->firstDateTime.msecsTo(d->currentDateTime);
                PositionData positionData;
                positionData.timestamp = timestamp;
                Flight &flight = Logbook::getInstance().getCurrentFlight();
                Position &position = flight.getUserAircraft().getPosition();
                position.upsert(std::move(positionData));
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


                    Flight &flight = Logbook::getInstance().getCurrentFlight();
                    Position &position = flight.getUserAircraft().getPosition();
                    position[d->currentPositionIndex].latitude = latitude;
                    position[d->currentPositionIndex].longitude = longitude;
                    position[d->currentPositionIndex].altitude = Convert::metersToFeet(altitude);
                    // TODO Calcualte feet/sec
                    position[d->currentPositionIndex].velocityBodyZ = 400;


                    ++d->currentPositionIndex;
                }


            } else {
                d->xml.raiseError(tr("Invalid GPS coordinate."));
            }
        } else {
            d->xml.skipCurrentElement();
        }
    }
}
