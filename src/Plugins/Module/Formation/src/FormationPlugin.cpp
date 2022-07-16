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

#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Settings.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/AircraftService.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include "Formation.h"
#include "FormationWidget.h"
#include "FormationPlugin.h"

struct FormationPluginPrivate
{
    FormationPluginPrivate(FlightService &flightService)
        : formationWidget(std::make_unique<FormationWidget>(flightService, *aircraftService))
    {}

    std::unique_ptr<AircraftService> aircraftService {std::make_unique<AircraftService>()};
    std::unique_ptr<FormationWidget> formationWidget;
};

// PUBLIC

FormationPlugin::FormationPlugin(QObject *parent) noexcept
    : AbstractModule(parent),
      d(std::make_unique<FormationPluginPrivate>(getFlightService()))
{
     Q_INIT_RESOURCE(FormationPlugin);
#ifdef DEBUG
    qDebug() << "FormationPlugin::FormationPlugin: CREATED.";
#endif
}

FormationPlugin::~FormationPlugin() noexcept
{
#ifdef DEBUG
    qDebug() << "FormationPlugin::~FormationPlugin: DELETED.";
#endif
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
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const Formation::HorizontalDistance horizontalDistance {d->formationWidget->getHorizontalDistance()};
    const Formation::VerticalDistance verticalDistance {d->formationWidget->getVerticalDistance()};
    const Formation::RelativePosition relativePosition {d->formationWidget->getRelativePosition()};
    // The initial recording position is calculated for timestamp = 0 ("at the beginning")
    const InitialPosition initialPosition = Settings::getInstance().isRelativePositionPlacementEnabled() ?
                Formation::calculateInitialRelativePositionToUserAircraft(horizontalDistance, verticalDistance, relativePosition, 0) :
                InitialPosition::NullData;
    skyConnectManager.startRecording(SkyConnectIntf::RecordingMode::AddToFormation, initialPosition);
}

void FormationPlugin::onStartReplay() noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const bool fromStart = skyConnectManager.isAtEnd();
    const Formation::HorizontalDistance horizontalDistance {d->formationWidget->getHorizontalDistance()};
    const Formation::VerticalDistance verticalDistance {d->formationWidget->getVerticalDistance()};
    const Formation::RelativePosition relativePosition {d->formationWidget->getRelativePosition()};
    const std::int64_t timestamp = fromStart ? 0 : skyConnectManager.getCurrentTimestamp();
    const InitialPosition initialPosition = Settings::getInstance().isRelativePositionPlacementEnabled() ?
        Formation::calculateInitialRelativePositionToUserAircraft(horizontalDistance, verticalDistance, relativePosition, timestamp) :
        InitialPosition::NullData;
    skyConnectManager.startReplay(fromStart, initialPosition);
}

// PROTECTED SLOTS

void FormationPlugin::onRecordingStopped() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    const std::size_t sequenceNumber = flight.count();
    if (sequenceNumber > 1) {
        // Sequence starts at 1
        d->aircraftService->store(flight.getId(), sequenceNumber, flight[sequenceNumber - 1]);
    } else {
        AbstractModule::onRecordingStopped();
    }
}