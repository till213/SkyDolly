/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Connect/Connect.h>
#include "AbstractSimulationVariableWidget.h"


AbstractSimulationVariableWidget  ::AbstractSimulationVariableWidget(QWidget *parent) noexcept
    : QWidget {parent}
{}

AbstractSimulationVariableWidget::~AbstractSimulationVariableWidget() = default;

// PROTECTED

void AbstractSimulationVariableWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);

    const auto &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::timestampChanged,
            this, &AbstractSimulationVariableWidget::updateUi);

    updateUi(skyConnectManager.getCurrentTimestamp(), TimeVariableData::Access::DiscreteSeek);

    auto &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::userAircraftChanged,
            this, &AbstractSimulationVariableWidget::updateUiWithCurrentTime);
}

void AbstractSimulationVariableWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);

    const auto &skyConnectManager = SkyConnectManager::getInstance();
    disconnect(&skyConnectManager, &SkyConnectManager::timestampChanged,
               this, &AbstractSimulationVariableWidget::updateUi);
    auto &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::userAircraftChanged,
               this, &AbstractSimulationVariableWidget::updateUiWithCurrentTime);
}

// PRIVATE SLOTS

void AbstractSimulationVariableWidget::updateUiWithCurrentTime() noexcept
{
    const auto &skyConnectManager = SkyConnectManager::getInstance();
    updateUi(skyConnectManager.getCurrentTimestamp(), TimeVariableData::Access::DiscreteSeek);
}
