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
#include <vector>
#include <cmath>

#include <QWidget>
#include <QIODevice>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QDateTime>
#include <QElapsedTimer>
#include <QCursor>
#include <QGuiApplication>

#include <Kernel/File.h>
#include <Kernel/Unit.h>
#include <Kernel/Settings.h>
#include <Kernel/SkyMath.h>
#include <Kernel/Convert.h>
#include <Model/Flight.h>
#include <Model/FlightData.h>
#include <Model/FlightCondition.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include <Flight/FlightAugmentation.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/AircraftService.h>
#include <Persistence/Service/AircraftTypeService.h>
#include "BasicFlightImportDialog.h"
#include "FlightImportPluginBaseSettings.h"
#include "FlightImportPluginBase.h"

struct FlightImportPluginBasePrivate
{
    Flight *flight {nullptr};
    std::unique_ptr<AircraftService> aircraftService {std::make_unique<AircraftService>()};
    std::unique_ptr<AircraftTypeService> aircraftTypeService {std::make_unique<AircraftTypeService>()};
    QFile file;
    Unit unit;
    AircraftType aircraftType;
    FlightAugmentation flightAugmentation;
};

// PUBLIC

FlightImportPluginBase::FlightImportPluginBase() noexcept
    : d(std::make_unique<FlightImportPluginBasePrivate>())
{}

FlightImportPluginBase::~FlightImportPluginBase() = default;

bool FlightImportPluginBase::importFlight(FlightService &flightService, Flight &flight) noexcept
{
    bool ok {false};
    d->flight = &flight;
    FlightImportPluginBaseSettings &baseSettings = getPluginSettings();
    std::unique_ptr<QWidget> optionWidget = createOptionWidget();
    std::unique_ptr<BasicFlightImportDialog> importDialog = std::make_unique<BasicFlightImportDialog>(flight, getFileFilter(), baseSettings, PluginBase::getParentWidget());
    // Transfer ownership to importDialog
    importDialog->setOptionWidget(optionWidget.release());
    const int choice = importDialog->exec();
    if (choice == QDialog::Accepted) {
        QStringList selectedFilePaths;
        // Remember import (export) path
        const QString selectedPath = importDialog->getSelectedPath();
        if (baseSettings.isImportDirectoryEnabled()) {
            Settings::getInstance().setExportPath(selectedPath);
            selectedFilePaths = File::getFilePaths(selectedPath, getFileExtension());
        } else {
            const QString directoryPath = QFileInfo(selectedPath).absolutePath();
            Settings::getInstance().setExportPath(directoryPath);
            selectedFilePaths.append(selectedPath);
        }
        d->aircraftType = importDialog->getSelectedAircraftType(&ok);
        if (ok) {
#ifdef DEBUG
            QElapsedTimer timer;
            timer.start();
#endif
            QGuiApplication::setOverrideCursor(Qt::WaitCursor);
            QGuiApplication::processEvents();
            ok = importFlights(selectedFilePaths, flightService, flight);
            QGuiApplication::restoreOverrideCursor();
#ifdef DEBUG
            qDebug() << QFileInfo(selectedPath).fileName() << "import" << (ok ? "SUCCESS" : "FAIL") << "in" << timer.elapsed() <<  "ms";
#endif
            if (!ok && !baseSettings.isImportDirectoryEnabled()) {
                QMessageBox::warning(PluginBase::getParentWidget(), tr("Import error"), tr("The file %1 could not be imported.").arg(selectedPath));
            }
        } else {
            QMessageBox::warning(PluginBase::getParentWidget(), tr("Import error"),
                                 tr("The selected aircraft '%1' is not a known aircraft in the logbook. "
                                    "Check for spelling errors or record a flight with this aircraft first.").arg(d->aircraftType.type));
        }
    } else {
        ok = true;
    }

    // We are done with the export
    d->flight = nullptr;

    return ok;
}

// PROTECTED

AircraftType &FlightImportPluginBase::getSelectedAircraftType() const noexcept
{
    return d->aircraftType;
}

// PRIVATE

void FlightImportPluginBase::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    getPluginSettings().addSettings(keyValues);
}

void FlightImportPluginBase::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    getPluginSettings().addKeysWithDefaults(keysWithDefaults);
}

void FlightImportPluginBase::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    getPluginSettings().restoreSettings(valuesByKey);
}

bool FlightImportPluginBase::importFlights(const QStringList &filePaths, FlightService &flightService, Flight &flight) noexcept
{
    const FlightImportPluginBaseSettings &pluginSettings = getPluginSettings();
    const bool importDirectory = pluginSettings.isImportDirectoryEnabled();
    const bool addToCurrentFlight = pluginSettings.isAddToFlightEnabled();
    std::vector<FlightData> importedFlights;

    bool ok {true};
    bool ignoreFailures {false};
    bool isFirstFile {true};
    for (const QString &filePath : filePaths) {
        d->file.setFileName(filePath);
        ok = d->file.open(QIODevice::ReadOnly);
        if (ok) {
            // Clear the current flight IF
            // - We don't want to add the imported aircraft to the current flight OR
            // - We import an entire directory, and this is the first file to be imported
            if (!addToCurrentFlight || (importDirectory && isFirstFile)) {
                flight.clear(true);
                isFirstFile = false;
            }

            importedFlights = importFlights(d->file, ok);
            if (!pluginSettings.hasLogbookSupport()) {
                // TODO IMPLEMENT ME REFACTOR ME
                // The flight has always at least one aircraft, but possibly without recording (when the flight has
                // been cleared / newly created)
//                const bool addNewAircraft = addToCurrentFlight && flight.getUserAircraft().hasRecording();
//                Aircraft &userAircraft = addNewAircraft ? flight.addUserAircraft() : flight.getUserAircraft();
                if (ok) {
                    for (auto &importedFlight : importedFlights) {
                        for (auto &aircraft : importedFlight) {
                            augmentAircraft(aircraft);
                            const std::size_t nofAircraft = flight.count();
                            if (nofAircraft > 1) {
                                // Sequence starts at 1
                                const std::size_t sequenceNumber {nofAircraft};
                                ok = d->aircraftService->store(flight.getId(), sequenceNumber, aircraft);
                            } else {
                                // Also update flight info and condition
                                updateFlightInfo();
                                updateFlightCondition();
                                ok = flightService.store(flight);
                            }
                        }
                    }
                }
                if (!ok) {
                    flight.removeLastAircraft();
                }
            }
            if (ok) {
                for (auto &flightData : importedFlights) {
                    Flight flight {std::move(flightData)};
                    // TODO Refactor flight sevice: store/restore FlightData (instead of Flight)
                    ok = flightService.store(flight);
                }
            }

            d->file.close();
        }

        if (!ok && importDirectory && !ignoreFailures) {
            QGuiApplication::restoreOverrideCursor();
            QFileInfo fileInfo {filePath};
            std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(PluginBase::getParentWidget());
            messageBox->setIcon(QMessageBox::Warning);
            QPushButton *proceedButton = messageBox->addButton(tr("&Proceed"), QMessageBox::AcceptRole);
            QPushButton *ignoreAllButton = messageBox->addButton(tr("&Ignore All Failures"), QMessageBox::YesRole);
            messageBox->setWindowTitle(tr("Import Failure"));
            messageBox->setText(tr("The file %1 could not be imported. Do you want to proceed with the remaining files in directory %2?").arg(fileInfo.fileName(), fileInfo.dir().dirName()));
            messageBox->setInformativeText(tr("Aborting will keep the already successfully imported flights and aircraft."));
            messageBox->setStandardButtons(QMessageBox::Cancel);
            messageBox->setDefaultButton(proceedButton);

            messageBox->exec();
            const QAbstractButton *clickedButton = messageBox->clickedButton();
            if (clickedButton == ignoreAllButton) {
                ignoreFailures = true;
            } else if (clickedButton != proceedButton) {
                break;
            }
            QGuiApplication::setOverrideCursor(Qt::WaitCursor);
            QGuiApplication::processEvents();
        }

    } // All files

    return ok;
}

void FlightImportPluginBase::updateAircraftInfo() noexcept
{
    Aircraft &aircraft = d->flight->getUserAircraft();
    AircraftInfo aircraftInfo(aircraft.getId());
    aircraftInfo.aircraftType = d->aircraftType;

    const Position &position = aircraft.getPosition();   
    if (position.count() > 0) {
        const PositionData &lastPositionData = position.getLast();
        const QDateTime startDateTimeUtc = getStartDateTimeUtc();
        const QDateTime endDateTimeUtc = startDateTimeUtc.addMSecs(lastPositionData.timestamp);

        const PositionData &firstPositionData = position.getFirst();
        aircraftInfo.initialAirspeed = static_cast<int>(std::round(Convert::feetPerSecondToKnots(firstPositionData.velocityBodyZ)));

        // Add default waypoints (first and last position) in case none are present in the imported data
        FlightPlan &flightPlan = aircraft.getFlightPlan();
        std::size_t waypointCount = flightPlan.count();
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
            // Make sure that waypoints have distinct timestamps, especially in the case when
            // the aircraft has only one sampled position ("remains parked")
            arrival.timestamp = firstPositionData.timestamp != lastPositionData.timestamp ? lastPositionData.timestamp : lastPositionData.timestamp + 1;
            flightPlan.add(std::move(arrival));
        }
    } else {
        aircraftInfo.initialAirspeed = 0.0;
    }
    updateExtendedAircraftInfo(aircraftInfo);
    aircraft.setAircraftInfo(aircraftInfo);
}

void FlightImportPluginBase::updateFlightInfo() noexcept
{
    d->flight->setTitle(getTitle());

    const QString description = tr("Aircraft imported on %1 from file: %2").arg(d->unit.formatDateTime(QDateTime::currentDateTime()), d->file.fileName());
    d->flight->setDescription(description);
    d->flight->setCreationTime(QFileInfo(d->file).birthTime());
    updateExtendedFlightInfo(*d->flight);
}

void FlightImportPluginBase::updateFlightCondition() noexcept
{
    FlightCondition flightCondition;

    Aircraft &aircraft = d->flight->getUserAircraft();

    const Position &position = aircraft.getPosition();
    const PositionData &lastPositionData = position.getLast();
    const QDateTime startDateTimeUtc = getStartDateTimeUtc();
    const QDateTime endDateTimeUtc = startDateTimeUtc.addMSecs(lastPositionData.timestamp);

    flightCondition.startLocalTime = startDateTimeUtc.toLocalTime();
    flightCondition.startZuluTime = startDateTimeUtc;
    flightCondition.endLocalTime = endDateTimeUtc.toLocalTime();
    flightCondition.endZuluTime = endDateTimeUtc;
    updateExtendedFlightCondition(flightCondition);

    d->flight->setFlightCondition(flightCondition);
}

bool FlightImportPluginBase::augmentAircraft(Aircraft &aircraft) noexcept
{
    bool ok {false};
    if (aircraft.getPosition().count() > 0) {
        d->flightAugmentation.setProcedures(getProcedures());
        d->flightAugmentation.setAspects(getAspects());
        d->flightAugmentation.augmentAircraftData(aircraft);
        updateAircraftInfo();
    }
    return ok;
}
