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
#include <QWidget>
#include <QAction>
#include <QUuid>
#include <QDir>
#include <QMessageBox>

#include <Kernel/Settings.h>
#include <Model/Logbook.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/PersistenceManager.h>
#include <Module/ModuleBaseSettings.h>
#include <Module/AbstractModule.h>
#include <Connect/SkyConnectIntf.h>
#include <SkyConnectManager.h>

struct AbstractModulePrivate
{
    std::unique_ptr<FlightService> flightService {std::make_unique<FlightService>()};
};

// PUBLIC

AbstractModule::AbstractModule(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<AbstractModulePrivate>())
{
    frenchConnection();
}

AbstractModule::~AbstractModule() = default;

ModuleIntf::RecordIconId AbstractModule::getRecordIconId() const noexcept
{
    return ModuleIntf::RecordIconId::Normal;
}

void AbstractModule::setRecording(bool enable) noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    switch (skyConnectManager.getState()) {
    case Connect::State::Recording:
        if (!enable) {
            skyConnectManager.stopRecording();
        }
        break;
    case Connect::State::RecordingPaused:
        if (enable) {
            // The record button also unpauses a paused recording
            setPaused(false);
        }
        break;
    default:
        if (enable) {
            onStartRecording();
        }
        break;
    }
}

void AbstractModule::setPaused(bool enable) noexcept
{
    onPause(enable);
}

void AbstractModule::setPlaying(bool enable) noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    if (skyConnectManager.isPaused() && enable) {
        // The play button also unpauses a paused replay
        setPaused(false);
    } else if (enable) {
        onStartReplay();
    } else {
        skyConnectManager.stopReplay();
    }
}

void AbstractModule::storeSettings(const QUuid &pluginUuid) const noexcept
{
    Settings::KeyValues settings;
    addSettings(settings);
    if (settings.size() > 0) {
        Settings::getInstance().storeModuleSettings(pluginUuid, settings);
    }
}

void AbstractModule::restoreSettings(const QUuid &pluginUuid) noexcept
{
    Settings::KeysWithDefaults keysWithDefaults;
    addKeysWithDefaults(keysWithDefaults);
    if (keysWithDefaults.size() > 0) {
        Settings::ValuesByKey values = Settings::getInstance().restoreModuleSettings(pluginUuid, keysWithDefaults);
        restoreSettings(values);
    }
}

// PROTECTED

void AbstractModule::onStartRecording() noexcept
{
    SkyConnectManager::getInstance().startRecording(SkyConnectIntf::RecordingMode::SingleAircraft);
}

void AbstractModule::onPause(bool enable) noexcept
{
    SkyConnectManager::getInstance().setPaused(SkyConnectIntf::Initiator::App, enable);
}

void AbstractModule::onStartReplay() noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    skyConnectManager.startReplay(skyConnectManager.isAtEnd());
}

// PROTECTED SLOTS

void AbstractModule::onRecordingStopped() noexcept
{
    const bool ok = d->flightService->storeFlight(Logbook::getInstance().getCurrentFlight());
    if (!ok) {
        const PersistenceManager &persistenceManager = PersistenceManager::getInstance();
        const QString logbookPath = QDir::toNativeSeparators(persistenceManager.getLogbookPath());
        QMessageBox::critical(getWidget(), tr("Flight error"), tr("The flight could not be stored into the logbook %1.").arg(logbookPath));
    }
}

void AbstractModule::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    getModuleSettings().addSettings(keyValues);
}

void AbstractModule::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    getModuleSettings().addKeysWithDefaults(keysWithDefaults);
}

void AbstractModule::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    getModuleSettings().restoreSettings(valuesByKey);
}

// PRIVATE

void AbstractModule::frenchConnection() noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::recordingStopped,
            this, &AbstractModule::onRecordingStopped);
}
