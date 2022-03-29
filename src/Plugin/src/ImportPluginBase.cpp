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

#include <QWidget>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QElapsedTimer>
#include <QCursor>
#include <QGuiApplication>

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
#include "../../Persistence/src/Service/AircraftTypeService.h"
#include "BasicImportDialog.h"
#include "ImportPluginBase.h"

class ImportPluginBasePrivate
{
public:
    ImportPluginBasePrivate()
        : aircraftService(std::make_unique<AircraftService>()),
          aircraftTypeService(std::make_unique<AircraftTypeService>()),
          addToCurrentFlight(false)
    {}

    std::unique_ptr<AircraftService> aircraftService;
    std::unique_ptr<AircraftTypeService> aircraftTypeService;
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

bool ImportPluginBase::importData(FlightService &flightService) noexcept
{
    bool ok;
    Settings &settings = Settings::getInstance();
    std::unique_ptr<QWidget> optionWidget = createOptionWidget();
    std::unique_ptr<BasicImportDialog> importDialog = std::make_unique<BasicImportDialog>(getFileFilter(), getParentWidget());
    connect(importDialog.get(), &BasicImportDialog::restoreDefaultOptions,
            this, &ImportPluginBase::onRestoreDefaultSettings);
    // Transfer ownership to importDialog
    importDialog->setOptionWidget(optionWidget.release());
    const int choice = importDialog->exec();
    if (choice == QDialog::Accepted) {
        // Remember import (export) path
        const QString selectedFilePath = importDialog->getSelectedFilePath();
        const QString filePath = QFileInfo(selectedFilePath).absolutePath();
        settings.setExportPath(filePath);
        ok = importDialog->getSelectedAircraftType(d->aircraftType);
        if (ok) {
            d->addToCurrentFlight = importDialog->isAddToFlightEnabled();
#ifdef DEBUG
            QElapsedTimer timer;
            timer.start();
#endif
            QGuiApplication::setOverrideCursor(Qt::WaitCursor);
            QGuiApplication::processEvents();
            ok = importFile(selectedFilePath, flightService);
            QGuiApplication::restoreOverrideCursor();
#ifdef DEBUG
            qDebug("%s import %s in %lld ms", qPrintable(QFileInfo(selectedFilePath).fileName()), (ok ? qPrintable("SUCCESS") : qPrintable("FAIL")), timer.elapsed());
#endif
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
            QMessageBox::critical(getParentWidget(), tr("Import error"),
                                  tr("The selected aircraft '%1' is not a known aircraft in the logbook. "
                                     "Check for spelling errors or record a flight with this aircraft first.").arg(d->aircraftType.type));
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
            d->flightAugmentation.setProcedures(getProcedures());
            d->flightAugmentation.setAspects(getAspects());
            d->flightAugmentation.augmentAircraftData(aircraft);
            updateAircraftInfo();
            if (addNewAircraft) {
                // Sequence starts at 1
                const int sequenceNumber = flight.count();
                ok = d->aircraftService->store(flight.getId(), sequenceNumber, aircraft);
            } else {
                // Also update flight info and condition
                updateFlightInfo();
                updateFlightCondition();
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
    const PositionData &lastPositionData = position.getLast();
    const QDateTime startDateTimeUtc = getStartDateTimeUtc();
    const QDateTime endDateTimeUtc = startDateTimeUtc.addMSecs(lastPositionData.timestamp);
    int positionCount = position.count();
    if (positionCount > 0) {
        const PositionData &firstPositionData = position.getFirst();
        aircraftInfo.initialAirspeed = Convert::feetPerSecondToKnots(firstPositionData.velocityBodyZ);

        // Add default waypoints (first and last position) in case none are present in the imported data
        FlightPlan &flightPlan = aircraft.getFlightPlan();
        int waypointCount = flightPlan.count();
        if (waypointCount == 0) {

            Waypoint departure;
            departure.identifier = Waypoint::CustomDepartureIdentifier;
            departure.latitude = static_cast<float>(firstPositionData.latitude);
            departure.longitude = static_cast<float>(firstPositionData.longitude);
            departure.altitude = static_cast<float>(firstPositionData.altitude);
            departure.localTime = startDateTimeUtc.toLocalTime();
            departure.zuluTime = startDateTimeUtc;
            departure.timestamp = firstPositionData.timestamp;
            flightPlan.add(std::move(departure));

            Waypoint arrival;
            arrival.identifier = Waypoint::CustomArrivalIdentifier;
            arrival.latitude = static_cast<float>(lastPositionData.latitude);
            arrival.longitude = static_cast<float>(lastPositionData.longitude);
            arrival.altitude = static_cast<float>(lastPositionData.altitude);
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

void ImportPluginBase::updateFlightInfo() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    flight.setTitle(getTitle());

    const QString description = tr("Aircraft imported on %1 from file: %2").arg(d->unit.formatDateTime(QDateTime::currentDateTime()), d->file.fileName());
    flight.setDescription(description);
    flight.setCreationDate(QFileInfo(d->file).birthTime());
    updateExtendedFlightInfo(flight);
}

void ImportPluginBase::updateFlightCondition() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    FlightCondition flightCondition;

    Aircraft &aircraft = flight.getUserAircraft();

    const Position &position = aircraft.getPositionConst();
    const PositionData &lastPositionData = position.getLast();
    const QDateTime startDateTimeUtc = getStartDateTimeUtc();
    const QDateTime endDateTimeUtc = startDateTimeUtc.addMSecs(lastPositionData.timestamp);

    flightCondition.startLocalTime = startDateTimeUtc.toLocalTime();
    flightCondition.startZuluTime = startDateTimeUtc;
    flightCondition.endLocalTime = endDateTimeUtc.toLocalTime();
    flightCondition.endZuluTime = endDateTimeUtc;
    updateExtendedFlightCondition(flightCondition);

    flight.setFlightCondition(flightCondition);
}
