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
    std::unique_ptr<FlightService> flightService {std::make_unique<FlightService>()};
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

bool FlightImportPluginBase::importFlights(Flight &flight) noexcept
{
    bool ok {false};
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
            ok = importFlights(selectedFilePaths, flight);
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

bool FlightImportPluginBase::importFlights(const QStringList &filePaths, Flight &currentFlight) noexcept
{
    const FlightImportPluginBaseSettings &pluginSettings = getPluginSettings();
    const bool importDirectory = pluginSettings.isImportDirectoryEnabled();
    const FlightImportPluginBaseSettings::AircraftImportMode aircraftImportMode = pluginSettings.getAircraftImportMode();

    if (aircraftImportMode == FlightImportPluginBaseSettings::AircraftImportMode::AddToNewFlight) {
        currentFlight.clear(true);
    }

    bool ok {true};
    bool ignoreAllFailures {false};
    bool continueWithDirectoryImport {true};
    std::vector<FlightData> importedFlightData;

    std::size_t totalFlightsStored {0};
    std::size_t totalAircraftStored {0};
    for (const QString &filePath : filePaths) {
        d->file.setFileName(filePath);
        ok = d->file.open(QIODevice::ReadOnly);
        if (ok) {
            importedFlightData = importSelectedFlights(d->file, ok);
            if (ok) {
                enrichFlightData(importedFlightData);
                if (getAugmentationAspects() || getAugmentationProcedures()) {
                    augmentFlights(importedFlightData);
                }

                switch (aircraftImportMode) {
                case FlightImportPluginBaseSettings::AircraftImportMode::AddToNewFlight:
                    [[fallthrough]];
                case FlightImportPluginBaseSettings::AircraftImportMode::AddToCurrentFlight:
                    ok = addAndStoreAircraftToCurrentFlight(filePath, std::move(importedFlightData), currentFlight,
                                                            totalFlightsStored, totalAircraftStored, continueWithDirectoryImport);
                    break;
                case FlightImportPluginBaseSettings::AircraftImportMode::SeparateFlights:
                    // Store all imported flight data into the logbook
                    ok = storeFlightData(importedFlightData, totalFlightsStored);
                    break;
                }
            }
            d->file.close();
        }

        if (!ok && importDirectory && !ignoreAllFailures) {
            confirmImportError(filePath, ignoreAllFailures, continueWithDirectoryImport);
        }

        if (!continueWithDirectoryImport) {
            break;
        }
    } // All files

    // Notify the application that...
    if (totalFlightsStored > 0) {
        // ...  at least one flight has been stored ...
        emit currentFlight.flightStored();
    } else if (totalAircraftStored > 0) {
        // ... or aircraft have been added to the current flight
        emit currentFlight.aircraftStored();
    }
    if (ok && aircraftImportMode == FlightImportPluginBaseSettings::AircraftImportMode::SeparateFlights) {
        // Load the last imported flight into the current flight
        FlightData &flightData = importedFlightData.back();
        currentFlight.fromFlightData(std::move(flightData));
    }

    return ok;
}

void FlightImportPluginBase::enrichFlightData(std::vector<FlightData> &flightData) const noexcept
{
    for (FlightData &flight : flightData) {
        enrichFlightCondition(flight);
        // Aircraft info depends on data from flight condition
        enrichAircraftInfo(flight);
        enrichFlightInfo(flight);
    }
}

void FlightImportPluginBase::enrichFlightInfo(FlightData &flightData) const noexcept
{
    if (!flightData.creationTime.isValid()) {
        flightData.creationTime = QFileInfo(d->file).birthTime();
    }
    if (flightData.title.isEmpty()) {
        flightData.title = tr("Imported %1").arg(flightData.aircraft.front().getAircraftInfo().aircraftType.type);
    }
    if (flightData.description.isEmpty()) {
        flightData.description = tr("Flight imported on %1 from file: %2").arg(d->unit.formatDateTime(QDateTime::currentDateTime()), d->file.fileName());
    }
}

void FlightImportPluginBase::enrichFlightCondition(FlightData &flightData) const noexcept
{
    FlightCondition &flightCondition = flightData.flightCondition;

    if (!(flightCondition.startLocalTime.isValid() && flightCondition.startZuluTime.isValid())) {
        flightCondition.startZuluTime = QDateTime::currentDateTimeUtc();
        flightCondition.startLocalTime = flightCondition.startZuluTime.toLocalTime();
    }

    if (!(flightCondition.endLocalTime.isValid() && flightCondition.endZuluTime.isValid())) {
        const Aircraft &aircraft = flightData.getUserAircraft();
        const Position &position = aircraft.getPosition();
        const PositionData &lastPositionData = position.getLast();
        flightCondition.endLocalTime =  flightCondition.startLocalTime.addMSecs(lastPositionData.timestamp);
        flightCondition.endZuluTime = flightCondition.startZuluTime.addMSecs(lastPositionData.timestamp);
    }
}

void FlightImportPluginBase::enrichAircraftInfo(FlightData &flightData) const noexcept
{
    for (Aircraft &aircraft : flightData) {
        AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();
        if (aircraftInfo.aircraftType.isNull()) {
            aircraftInfo.aircraftType = d->aircraftType;
        }

        const Position &position = aircraft.getPosition();
        if (position.count() > 0) {
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
                departure.localTime = flightData.flightCondition.startLocalTime;
                departure.zuluTime = flightData.flightCondition.startZuluTime;
                departure.timestamp = firstPositionData.timestamp;
                flightPlan.add(std::move(departure));

                const PositionData &lastPositionData = position.getLast();
                Waypoint arrival;
                arrival.identifier = Waypoint::CustomArrivalIdentifier;
                arrival.latitude = static_cast<float>(lastPositionData.latitude);
                arrival.longitude = static_cast<float>(lastPositionData.longitude);
                arrival.altitude = static_cast<float>(lastPositionData.altitude);
                arrival.localTime = flightData.flightCondition.endLocalTime;
                arrival.zuluTime = flightData.flightCondition.endZuluTime;
                // Make sure that waypoints have distinct timestamps, especially in the case when
                // the aircraft has only one sampled position ("remains parked")
                arrival.timestamp = firstPositionData.timestamp != lastPositionData.timestamp ? lastPositionData.timestamp : lastPositionData.timestamp + 1;
                flightPlan.add(std::move(arrival));
            }
        } else {
            aircraftInfo.initialAirspeed = 0.0;
        }
    }
}

bool FlightImportPluginBase::augmentFlights(std::vector<FlightData> &flightData) const noexcept
{
    bool ok {false};
    for (FlightData &flight : flightData) {
        for (Aircraft &aircraft : flight) {
            if (aircraft.getPosition().count() > 0) {
                d->flightAugmentation.setProcedures(getAugmentationProcedures());
                d->flightAugmentation.setAspects(getAugmentationAspects());
                d->flightAugmentation.augmentAircraftData(aircraft);
            }
        }
    }
    return ok;
}

bool FlightImportPluginBase::addAndStoreAircraftToCurrentFlight(const QString sourceFilePath, std::vector<FlightData> importedFlightData, Flight &currentFlight,
                                                                std::size_t &totalFlightsStored, std::size_t &totalAircraftStored, bool &continueWithDirectoryImport) noexcept
{
    bool ok {true};
    bool newFlight = !currentFlight.hasRecording();
    bool doAdd {true};
    if (importedFlightData.size() > 1) {
        confirmMultiFlightImport(sourceFilePath, importedFlightData.size(), doAdd, continueWithDirectoryImport);
    }
    if (doAdd) {
        // Iterate over all imported flights (if multiple)...
        for (auto &flightData : importedFlightData) {
            if (newFlight) {
                // Create a new flight first, based on the first imported flight data
                currentFlight.fromFlightData(std::move(flightData));
                ok = d->flightService->storeFlight(currentFlight);
                if (ok) {
                    ++totalFlightsStored;
                    newFlight = false;
                }
            } else {
                // Keep on adding the imported aircraft to current flight (sequence number starts at 1)
                std::size_t sequenceNumber = currentFlight.count() + 1;
                for (auto &aircraft : flightData.aircraft) {
                    ok = d->aircraftService->store(currentFlight.getId(), sequenceNumber, aircraft);
                    if (ok) {
                        ++sequenceNumber;
                        ++totalAircraftStored;
                    } else {
                        break;
                    }
                }
                if (ok) {
                    currentFlight.addAircraft(std::move(flightData.aircraft));
                }
            }            
        } // flight data
    }
    return ok;
}

bool FlightImportPluginBase::storeFlightData(std::vector<FlightData> &importedFlightData, std::size_t &totalFlightsStored)
{
    bool ok {false};
    for (auto &flightData : importedFlightData) {
        ok = d->flightService->storeFlightData(flightData);
        if (ok) {
            ++totalFlightsStored;
        } else {
            break;
        }
    }
    return ok;
}

void FlightImportPluginBase::confirmImportError(const QString &filePath, bool &ignoreAllFailures, bool &continueWithDirectoryImport) noexcept
{
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
        ignoreAllFailures = true;
        continueWithDirectoryImport = true;
    } else if (clickedButton == proceedButton) {
        ignoreAllFailures = false;
        continueWithDirectoryImport = true;
    } else {
        ignoreAllFailures = false;
        continueWithDirectoryImport = false;
    }
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QGuiApplication::processEvents();
}

void FlightImportPluginBase::confirmMultiFlightImport(const QString &sourceFilePath, std::size_t nofFlights, bool &doAdd, bool &continueWithDirectoryImport)
{
    QGuiApplication::restoreOverrideCursor();
    QFileInfo fileInfo {sourceFilePath};
    std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(PluginBase::getParentWidget());
    messageBox->setIcon(QMessageBox::Warning);
    QPushButton *proceedButton = messageBox->addButton(tr("&Add Aircraft From All Flights"), QMessageBox::AcceptRole);
    QPushButton *skipButton = messageBox->addButton(tr("&Skip This File"), QMessageBox::RejectRole);
    messageBox->setWindowTitle(tr("Multiple Flights"));
    messageBox->setText(tr("The file %1 contains more than one flight (number of flights: %2). Do you want to add all aircraft from this file to the current flight?")
                        .arg(fileInfo.fileName())
                        .arg(nofFlights));
    messageBox->setInformativeText(tr("The aircraft may have been recorded in completely different locations or at different time of day."));
    messageBox->setStandardButtons(QMessageBox::Cancel);
    messageBox->setDefaultButton(proceedButton);

    messageBox->exec();
    const QAbstractButton *clickedButton = messageBox->clickedButton();
    if (clickedButton == proceedButton) {
        continueWithDirectoryImport = true;
        doAdd = true;
    } else if (clickedButton == skipButton) {
        continueWithDirectoryImport = true;
        doAdd = false;
    } else {
        doAdd = false;
        continueWithDirectoryImport = false;
    }
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QGuiApplication::processEvents();
}
