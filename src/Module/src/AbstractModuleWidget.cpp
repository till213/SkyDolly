/**
 * Sky Dolly - The black sheep for your flight recordings
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

// PROTECTED

FlightService &AbstractModuleWidget::getFlightService() const noexcept
{
    return d->flightService;
}

// PROTECTED SLOTS

void AbstractModuleWidget::handleRecordingStopped() noexcept
{
    d->flightService.store(Logbook::getInstance().getCurrentFlight());
}
