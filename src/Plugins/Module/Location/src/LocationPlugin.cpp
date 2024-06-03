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
#include <QCoreApplication>

#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/Service/EnumerationService.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "LocationWidget.h"
#include "LocationSettings.h"
#include "LocationPlugin.h"

struct LocationPluginPrivate
{
    enum struct Mode
    {
        Add,
        Update
    };

    LocationSettings moduleSettings;
    std::unique_ptr<LocationWidget> locationWidget {std::make_unique<LocationWidget>(moduleSettings)};
    const std::int64_t EngineEventStartId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventStartSymId).id()};
    const std::int64_t EngineEventStopId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventStopSymId).id()};
    Mode mode {Mode::Add};
};

// PUBLIC

LocationPlugin::LocationPlugin(QObject *parent) noexcept
    : AbstractModule {parent},
      d {std::make_unique<LocationPluginPrivate>()}
{
    frenchConnection();
    restoreSettings(QUuid(Const::LocationModuleUuid));
}

LocationPlugin::~LocationPlugin()
{
    storeSettings(QUuid(Const::LocationModuleUuid));
};

QUuid LocationPlugin::getUuid() const noexcept
{
    static const QUuid uuid {Const::LocationModuleUuid};
    return uuid;
}

QString LocationPlugin::getModuleName() const noexcept
{
    return QCoreApplication::translate("LocationPlugin", "Location");
}

QWidget *LocationPlugin::getWidget() const noexcept
{
    return d->locationWidget.get();
}

// PROTECTED

ModuleBaseSettings &LocationPlugin::getModuleSettings() const noexcept
{
    return d->moduleSettings;
}

// PRIVATE

void LocationPlugin::frenchConnection() noexcept
{
    // Connection
    auto &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::locationReceived,
            this, &LocationPlugin::onLocationReceived);

    // Location widget
    connect(d->locationWidget.get(), &LocationWidget::doCaptureLocation,
            this, &LocationPlugin::captureLocation);
    connect(d->locationWidget.get(), &LocationWidget::doUpdateLocation,
            this, &LocationPlugin::updateLocation);
    connect(d->locationWidget.get(), &LocationWidget::teleportTo,
            this, &LocationPlugin::teleportTo);
}

// PRIVATE SLOTS

void LocationPlugin::captureLocation() noexcept
{
    d->mode = LocationPluginPrivate::Mode::Add;
    SkyConnectManager::getInstance().requestLocation();
}

void LocationPlugin::updateLocation() noexcept
{
    d->mode = LocationPluginPrivate::Mode::Update;
    SkyConnectManager::getInstance().requestLocation();
}

void LocationPlugin::teleportTo(const Location &location) noexcept
{
    const InitialPosition initialPosition = location.toInitialPosition();
    auto &skyConnectManager = SkyConnectManager::getInstance();
    skyConnectManager.setUserAircraftInitialPosition(initialPosition);
    SkyConnectIntf::SimulationEvent event {SkyConnectIntf::SimulationEvent::None};

    if (location.engineEventId == d->EngineEventStartId) {
        event = SkyConnectIntf::SimulationEvent::EngineStart;
    } else if (location.engineEventId == d->EngineEventStopId) {
        event = SkyConnectIntf::SimulationEvent::EngineStop;
    }

    if (event != SkyConnectIntf::SimulationEvent::None) {
        skyConnectManager.sendSimulationEvent(event);
    }
}

void LocationPlugin::onLocationReceived(Location location) noexcept
{
    switch (d->mode)
    {
    case LocationPluginPrivate::Mode::Add:
        d->locationWidget->addLocation(std::move(location));
        break;
    case LocationPluginPrivate::Mode::Update:
        d->locationWidget->updateLocation(location);
        break;
    }
}
