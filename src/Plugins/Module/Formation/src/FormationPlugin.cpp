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

#include <QObject>
#include <QUuid>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

#include <Kernel/Const.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Persistence/Service/AircraftService.h>
#include <Persistence/PersistenceManager.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "Formation.h"
#include "FormationWidget.h"
#include "FormationSettings.h"
#include "FormationPlugin.h"

struct FormationPluginPrivate
{
    FormationSettings moduleSettings;
    std::unique_ptr<AircraftService> aircraftService {std::make_unique<AircraftService>()};
    std::unique_ptr<FormationWidget> formationWidget {std::make_unique<FormationWidget>(moduleSettings)};
};

// PUBLIC

FormationPlugin::FormationPlugin(QObject *parent) noexcept
    : AbstractModule {parent},
      d {std::make_unique<FormationPluginPrivate>()}
{
    restoreSettings(QUuid(Const::FormationModuleUuid));
}

FormationPlugin::~FormationPlugin()
{
    storeSettings(QUuid(Const::FormationModuleUuid));
};

QUuid FormationPlugin::getUuid() const noexcept
{
    static const QUuid uuid {Const::FormationModuleUuid};
    return uuid;
}

QString FormationPlugin::getModuleName() const noexcept
{
    return QCoreApplication::translate("FormationPlugin", "Formation");
}

QWidget *FormationPlugin::getWidget() const noexcept
{
    return d->formationWidget.get();
}

ModuleIntf::RecordIconId FormationPlugin::getRecordIconId() const noexcept
{
    return ModuleIntf::RecordIconId::Add;
}

// PROTECTED

void FormationPlugin::onStartRecording() noexcept
{
    auto &skyConnectManager = SkyConnectManager::getInstance();
    const Formation::HorizontalDistance horizontalDistance {d->formationWidget->getHorizontalDistance()};
    const Formation::VerticalDistance verticalDistance {d->formationWidget->getVerticalDistance()};
    const Formation::Bearing relativePosition {d->formationWidget->getRelativePosition()};
    // The initial recording position is calculated for timestamp = 0 ("at the beginning")
    const InitialPosition initialPosition = d->moduleSettings.isRelativePositionPlacementEnabled() ?
        Formation::calculateInitialRelativePositionToUserAircraft(horizontalDistance, verticalDistance, relativePosition, 0) :
        InitialPosition();
    skyConnectManager.startRecording(SkyConnectIntf::RecordingMode::AddToFormation, initialPosition);
}

void FormationPlugin::onStartReplay() noexcept
{
    auto &skyConnectManager = SkyConnectManager::getInstance();
    const bool fromStart = skyConnectManager.isAtEnd();
    const Formation::HorizontalDistance horizontalDistance {d->formationWidget->getHorizontalDistance()};
    const Formation::VerticalDistance verticalDistance {d->formationWidget->getVerticalDistance()};
    const Formation::Bearing relativePosition {d->formationWidget->getRelativePosition()};
    const std::int64_t timestamp = fromStart ? 0 : skyConnectManager.getCurrentTimestamp();
    const InitialPosition initialPosition = d->moduleSettings.isRelativePositionPlacementEnabled() ?
        Formation::calculateInitialRelativePositionToUserAircraft(horizontalDistance, verticalDistance, relativePosition, timestamp) :
        InitialPosition();
    skyConnectManager.startReplay(fromStart, initialPosition);
}

ModuleBaseSettings &FormationPlugin::getModuleSettings() const noexcept
{
    return d->moduleSettings;
}

// PROTECTED SLOTS

void FormationPlugin::onRecordingStopped() noexcept
{
    auto &flight = Logbook::getInstance().getCurrentFlight();
    const auto sequenceNumber = flight.count();
    if (sequenceNumber > 1) {
        // Sequence starts at 1
        const bool ok = d->aircraftService->store(flight.getId(), sequenceNumber, flight[sequenceNumber - 1]);
        if (!ok) {
            flight.removeLastAircraft();
            const auto &persistenceManager = PersistenceManager::getInstance();
            const QString logbookPath = QDir::toNativeSeparators(persistenceManager.getLogbookPath());
            QMessageBox::critical(getWidget(), tr("Write Error"), tr("The aircraft could not be stored into the logbook %1.").arg(logbookPath));
        }
    } else {
        AbstractModule::onRecordingStopped();
    }
}
