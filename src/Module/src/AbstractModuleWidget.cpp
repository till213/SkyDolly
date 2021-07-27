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
#include <QWidget>
#include <QMessageBox>
#include <QAction>

#include "../../Model/src/Logbook.h"
#include "../../SkyConnect/src/SkyConnectManager.h"
#include "../../SkyConnect/src/SkyConnectIntf.h"
#include "../../Persistence/src/Service/FlightService.h"
#include "Module.h"
#include "AbstractModuleWidget.h"

class AbstractModuleWidgetPrivate
{
public:
    AbstractModuleWidgetPrivate(FlightService &theFlightService) noexcept
        : active(false),
          flightService(theFlightService)
    {}

    bool active;
    FlightService &flightService;
};

// PUBLIC

AbstractModuleWidget::AbstractModuleWidget(FlightService &flightService, QWidget *parent) noexcept
    : QWidget(parent),
      d(std::make_unique<AbstractModuleWidgetPrivate>(flightService))
{}

AbstractModuleWidget::~AbstractModuleWidget() noexcept
{}

bool AbstractModuleWidget::isActive() const noexcept
{
    return d->active;
}

void AbstractModuleWidget::setActive(bool enable) noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    if (enable) {
        connect(&skyConnectManager, &SkyConnectManager::recordingStopped,
                this, &AbstractModuleWidget::handleRecordingStopped);
    } else {
        disconnect(&skyConnectManager, &SkyConnectManager::recordingStopped,
                   this, &AbstractModuleWidget::handleRecordingStopped);
    }
    getAction().setChecked(enable);
    d->active = enable;
}

QWidget &AbstractModuleWidget::getWidget() noexcept
{
    return *this;
}

void AbstractModuleWidget::setRecording(bool enable) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        blockSignals(true);
        switch (skyConnect->get().getState()) {
        case Connect::State::Recording:
            if (!enable) {
                skyConnect->get().stopRecording();
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
        blockSignals(false);
    }
}

void AbstractModuleWidget::setPaused(bool enable) noexcept
{
    onPaused(enable);
}

void AbstractModuleWidget::setPlaying(bool enable) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().isPaused() && enable) {
            // The play button also unpauses a paused replay
            setPaused(false);
        } else if (enable) {
            onStartReplay();
        } else {
            skyConnect->get().stopReplay();
        }
    }
}

// PROTECTED

FlightService &AbstractModuleWidget::getFlightService() const noexcept
{
    return d->flightService;
}

void AbstractModuleWidget::onStartRecording() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().startRecording(SkyConnectIntf::RecordingMode::SingleAircraft);
    }
}

void AbstractModuleWidget::onPaused(bool enable) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().setPaused(enable);
    }
}

void AbstractModuleWidget::onStartReplay() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().startReplay(skyConnect->get().isAtEnd());
    }
}

// PROTECTED SLOTS

void AbstractModuleWidget::handleRecordingStopped() noexcept
{
    d->flightService.store(Logbook::getInstance().getCurrentFlight());
}
