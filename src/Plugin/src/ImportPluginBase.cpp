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

#include <QStringBuilder>
#include <QIODevice>
#include <QFlags>
#include <QByteArray>
#include <QList>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <QDateTime>
#include <QTimeZone>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringView>

#include "../../Kernel/src/Unit.h"
#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/SkyMath.h"
#include "../../Kernel/src/Convert.h"
#include "../../Flight/src/FlightAugmentation.h"
#include "../../Model/src/Logbook.h"
#include "../../Model/src/Flight.h"
#include "../../Model/src/FlightCondition.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/Position.h"
#include "../../Model/src/PositionData.h"
#include "../../Model/src/FlightPlan.h"
#include "../../Model/src/Waypoint.h"
#include "../../SkyConnect/src/SkyConnectManager.h"
#include "../../SkyConnect/src/SkyConnectIntf.h"
#include "../../Persistence/src/Service/FlightService.h"
#include "../../Persistence/src/Service/AircraftService.h"
#include "../../Widget/src/BasicImportDialog.h"
#include "ImportPluginBase.h"

class ImportPluginBasePrivate
{
public:
    ImportPluginBasePrivate()
        : aircraftService(std::make_unique<AircraftService>()),
          addToCurrentFlight(false)
    {}

    std::unique_ptr<AircraftService> aircraftService;
    QFile file;
    Unit unit;
    AircraftType aircraftType;
    bool addToCurrentFlight;
    FlightAugmentation flightAugmentation;
};

// PUBLIC

ImportPluginBase::ImportPluginBase() noexcept
    : d(std::make_unique<ImportPluginBasePrivate>())
{
#ifdef DEBUG
    qDebug("ImportPluginBase::ImportPluginBase: CREATED");
#endif
}

ImportPluginBase::~ImportPluginBase() noexcept
{
#ifdef DEBUG
    qDebug("ImportPluginBase::~ImportPluginBase: DELETED");
#endif
}

bool ImportPluginBase::import(FlightService &flightService) noexcept
{
    bool ok;
    std::unique_ptr<BasicImportDialog> importDialog = std::make_unique<BasicImportDialog>(getParentWidget());
    const int choice = importDialog->exec();
    if (choice == QDialog::Accepted) {
        // Remember import (export) path
        const QString selectedFilePath = importDialog->getSelectedFilePath();
        const QString filePath = QFileInfo(selectedFilePath).absolutePath();
        Settings::getInstance().setExportPath(filePath);
        ok = importDialog->getSelectedAircraftType(d->aircraftType);
        if (ok) {
            d->addToCurrentFlight = importDialog->isAddToFlightEnabled();
            ok = importFile(selectedFilePath, flightService);
            if (ok) {
                if (d->addToCurrentFlight) {
                    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
                    if (skyConnect) {
                        skyConnect->get().updateAIObjects();
                    }
                }
            } else {
                QMessageBox::critical(getParentWidget(), tr("Import error"), tr("The file %1 could not be imported.").arg(selectedFilePath));
            }
        } else {
            QMessageBox::critical(getParentWidget(), tr("Import error"), tr("The aircraft could not be selected."));
        }
    } else {
        ok = true;
    }
    return ok;
}

// PROTECTED

AircraftType &ImportPluginBase::getSelectedAircraftType() const noexcept
{
    return d->aircraftType;
}

// PRIVATE

bool ImportPluginBase::importFile(const QString &filePath, FlightService &flightService) noexcept
{
    d->file.setFileName(filePath);
    bool ok = d->file.open(QIODevice::ReadOnly);
    if (ok) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        if (!d->addToCurrentFlight) {
            flight.clear(true);
        }
        // The flight has at least one aircraft, but possibly without recording
        const int aircraftCount = flight.count();
        const bool addNewAircraft = d->addToCurrentFlight && (aircraftCount > 1 || flight.getUserAircraft().hasRecording());
        Aircraft &aircraft = addNewAircraft ? flight.addUserAircraft() : flight.getUserAircraft();

        ok = readFile(d->file);
        if (ok && aircraft.getPositionConst().count() > 0) {
            d->flightAugmentation.augmentAircraftData(aircraft);
            updateAircraftInfo();
            if (addNewAircraft) {
                // Sequence starts at 1
                const int sequenceNumber = flight.count();
                ok = d->aircraftService->store(flight.getId(), sequenceNumber, aircraft);
            } else {
                // Also update flight info and condition
                updateFlight(d->file);
                ok = flightService.store(flight);
            }
        } else {
            ok = false;
        }
        d->file.close();
    }
    return ok;
}

void ImportPluginBase::updateAircraftInfo() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Aircraft &aircraft = flight.getUserAircraft();
    AircraftInfo aircraftInfo(aircraft.getId());
    aircraftInfo.aircraftType = d->aircraftType;

    const Position &position = aircraft.getPositionConst();
    const QDateTime startDateTimeUtc = getStartDateTimeUtc();
    const QDateTime endDateTimeUtc = aircraftInfo.startDate.addMSecs(position.getLast().timestamp);
    aircraftInfo.startDate = startDateTimeUtc.toLocalTime();
    aircraftInfo.endDate = endDateTimeUtc;
    int positionCount = aircraft.getPosition().count();
    if (positionCount > 0) {
        const PositionData &firstPositionData = aircraft.getPosition().getFirst();
        aircraftInfo.initialAirspeed = Convert::feetPerSecondToKnots(firstPositionData.velocityBodyZ);

        // Add default waypoints (first and last position) in case none are present in the imported data
        FlightPlan &flightPlan = aircraft.getFlightPlan();
        int waypointCount = flightPlan.count();
        if (waypointCount == 0) {

            Waypoint departure;

            departure.identifier = Waypoint::CustomDepartureIdentifier;
            departure.latitude = firstPositionData.latitude;
            departure.longitude = firstPositionData.longitude;
            departure.altitude = firstPositionData.altitude;
            departure.localTime = startDateTimeUtc.toLocalTime();
            departure.zuluTime = startDateTimeUtc;
            departure.timestamp = firstPositionData.timestamp;
            flightPlan.add(std::move(departure));

            Waypoint arrival;
            const PositionData &lastPositionData = aircraft.getPosition().getLast();
            arrival.identifier = Waypoint::CustomArrivalIdentifier;
            arrival.latitude = lastPositionData.latitude;
            arrival.longitude = lastPositionData.longitude;
            arrival.altitude = lastPositionData.altitude;
            arrival.localTime = endDateTimeUtc.toLocalTime();
            arrival.zuluTime = endDateTimeUtc;
            arrival.timestamp = lastPositionData.timestamp;
            flightPlan.add(std::move(arrival));
        }
    } else {
        aircraftInfo.initialAirspeed = 0.0;
    }
    updateExtendedAircraftInfo(aircraftInfo);
    aircraft.setAircraftInfo(aircraftInfo);
}
