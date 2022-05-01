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
#include <vector>

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

#include "../../Kernel/src/File.h"
#include "../../Kernel/src/Unit.h"
#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/SkyMath.h"
#include "../../Kernel/src/Convert.h"
#include "../../Flight/src/FlightAugmentation.h"
#include "../../Model/src/Flight.h"
#include "../../Model/src/FlightCondition.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/Position.h"
#include "../../Model/src/PositionData.h"
#include "../../Model/src/FlightPlan.h"
#include "../../Model/src/Waypoint.h"
#include "../../PluginManager/src/SkyConnectManager.h"
#include "../../PluginManager/src/SkyConnectIntf.h"
#include "../../Persistence/src/Service/FlightService.h"
#include "../../Persistence/src/Service/AircraftService.h"
#include "../../Persistence/src/Service/AircraftTypeService.h"
#include "BasicImportDialog.h"
#include "ImportPluginBaseSettings.h"
#include "ImportPluginBase.h"

class ImportPluginBasePrivate
{
public:
    ImportPluginBasePrivate()
        : flight(nullptr),
          aircraftService(std::make_unique<AircraftService>()),
          aircraftTypeService(std::make_unique<AircraftTypeService>())
    {}

    Flight *flight;
    std::unique_ptr<AircraftService> aircraftService;
    std::unique_ptr<AircraftTypeService> aircraftTypeService;
    QFile file;
    Unit unit;
    AircraftType aircraftType;
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

bool ImportPluginBase::importFlight(FlightService &flightService, Flight &flight) noexcept
{
    bool ok;
    d->flight = &flight;
    ImportPluginBaseSettings &baseSettings = getPluginSettings();
    std::unique_ptr<QWidget> optionWidget = createOptionWidget();
    std::unique_ptr<BasicImportDialog> importDialog = std::make_unique<BasicImportDialog>(flight, getFileFilter(), baseSettings, getParentWidget());
    // Transfer ownership to importDialog
    importDialog->setOptionWidget(optionWidget.release());
    const int choice = importDialog->exec();
    if (choice == QDialog::Accepted) {
        QStringList selectedFilePaths;
        // Remember import (export) path
        const QString selectedPath = importDialog->getSelectedPath();
        if (baseSettings.isImportDirectoryEnabled()) {
            Settings::getInstance().setExportPath(selectedPath);
            selectedFilePaths = File::getFilePaths(selectedPath, getFileSuffix());
        } else {
            const QString directoryPath = QFileInfo(selectedPath).absolutePath();
            Settings::getInstance().setExportPath(directoryPath);
            selectedFilePaths.append(selectedPath);
        }
        ok = importDialog->getSelectedAircraftType(d->aircraftType);
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
            qDebug("%s import %s in %lld ms", qPrintable(QFileInfo(selectedPath).fileName()), (ok ? qPrintable("SUCCESS") : qPrintable("FAIL")), timer.elapsed());
#endif
            if (ok) {
                if (baseSettings.isAddToFlightEnabled()) {
                    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
                    if (skyConnect) {
                        skyConnect->get().updateAIObjects();
                    }
                }
            } else if (!baseSettings.isImportDirectoryEnabled()) {
                QMessageBox::warning(getParentWidget(), tr("Import error"), tr("The file %1 could not be imported.").arg(selectedPath));
            }
        } else {
            QMessageBox::warning(getParentWidget(), tr("Import error"),
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

AircraftType &ImportPluginBase::getSelectedAircraftType() const noexcept
{
    return d->aircraftType;
}

// PRIVATE

void ImportPluginBase::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    getPluginSettings().addSettings(keyValues);
}

void ImportPluginBase::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    getPluginSettings().addKeysWithDefaults(keysWithDefaults);
}

void ImportPluginBase::restoreSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    getPluginSettings().restoreSettings(valuesByKey);
}

bool ImportPluginBase::importFlights(const QStringList &filePaths, FlightService &flightService, Flight &flight) noexcept
{
    const ImportPluginBaseSettings &pluginSettings = getPluginSettings();
    const bool importDirectory = pluginSettings.isImportDirectoryEnabled();
    const bool addToCurrentFlight = pluginSettings.isAddToFlightEnabled();

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
            // The flight has always at least one aircraft, but possibly without recording (when the flight has
            // been cleared / newly created)
            const bool addNewAircraft = addToCurrentFlight && flight.getUserAircraft().hasRecording();
            Aircraft &aircraft = addNewAircraft ? flight.addUserAircraft() : flight.getUserAircraft();

            ok = importFlight(d->file, flight);
            if (ok && aircraft.getPositionConst().count() > 0) {
                d->flightAugmentation.setProcedures(getProcedures());
                d->flightAugmentation.setAspects(getAspects());
                d->flightAugmentation.augmentAircraftData(aircraft);
                updateAircraftInfo();
                const int nofAircraft = flight.count();
                if (nofAircraft > 1) {
                    // Sequence starts at 1
                    const int sequenceNumber = nofAircraft;
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

        if (!ok && importDirectory && !ignoreFailures) {
            QGuiApplication::restoreOverrideCursor();
            QFileInfo fileInfo {filePath};
            std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(getParentWidget());
            messageBox->setIcon(QMessageBox::Warning);
            QPushButton *proceedButton = messageBox->addButton(tr("&Proceed"), QMessageBox::AcceptRole);
            QPushButton *ignoreAllButton = messageBox->addButton(tr("&Ignore All Failures"), QMessageBox::YesRole);
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

void ImportPluginBase::updateAircraftInfo() noexcept
{
    Aircraft &aircraft = d->flight->getUserAircraft();
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
    d->flight->setTitle(getTitle());

    const QString description = tr("Aircraft imported on %1 from file: %2").arg(d->unit.formatDateTime(QDateTime::currentDateTime()), d->file.fileName());
    d->flight->setDescription(description);
    d->flight->setCreationTime(QFileInfo(d->file).birthTime());
    updateExtendedFlightInfo(*d->flight);
}

void ImportPluginBase::updateFlightCondition() noexcept
{
    FlightCondition flightCondition;

    Aircraft &aircraft = d->flight->getUserAircraft();

    const Position &position = aircraft.getPositionConst();
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
