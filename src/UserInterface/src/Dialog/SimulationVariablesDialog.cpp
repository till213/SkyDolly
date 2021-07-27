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

#include <QDialog>

#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/PrimaryFlightControl.h"
#include "../../../Model/src/AircraftInfo.h"
#include "../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "../Widget/AircraftWidget.h"
#include "../Widget/EngineWidget.h"
#include "../Widget/PrimaryFlightControlWidget.h"
#include "../Widget/SecondaryFlightControlWidget.h"
#include "../Widget/AircraftHandleWidget.h"
#include "../Widget/LightWidget.h"
#include "SimulationVariablesDialog.h"
#include "ui_SimulationVariablesDialog.h"

class SimulationVariablesDialogPrivate
{
public:
    SimulationVariablesDialogPrivate() noexcept
    {}

    static const char WindowTitle[];
};

const char SimulationVariablesDialogPrivate::WindowTitle[] = QT_TRANSLATE_NOOP("SimulationVariablesDialog", "Simulation Variables");

// PUBLIC

SimulationVariablesDialog::SimulationVariablesDialog(QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<SimulationVariablesDialogPrivate>()),
    ui(std::make_unique<Ui::SimulationVariablesDialog>())
{
    ui->setupUi(this);
    initUi();
}

SimulationVariablesDialog::~SimulationVariablesDialog() noexcept
{
#ifdef DEBUG
    qDebug("SimulationVariablesDialog::~SimulationVariablesDialog: DELETED");
#endif
}

// PROTECTED

void SimulationVariablesDialog::showEvent(QShowEvent *event) noexcept
{
    QDialog::showEvent(event);
    updateUi();
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &SimulationVariablesDialog::updateTitle);

    emit visibilityChanged(true);
}

void SimulationVariablesDialog::hideEvent(QHideEvent *event) noexcept
{
    QDialog::hideEvent(event);
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    disconnect(&skyConnectManager, &SkyConnectManager::stateChanged,
               this, &SimulationVariablesDialog::updateTitle);

    emit visibilityChanged(false);
}

// PRIVATE

void SimulationVariablesDialog::initUi() noexcept
{
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    AircraftWidget *aircraftWidget = new AircraftWidget(this);
    ui->simulationVariablesTab->addTab(aircraftWidget, tr("&Aircraft"));

    EngineWidget *engineWidget = new EngineWidget(this);
    ui->simulationVariablesTab->addTab(engineWidget, tr("&Engine"));

    PrimaryFlightControlWidget *primaryFlightControlWidget = new PrimaryFlightControlWidget(this);
    ui->simulationVariablesTab->addTab(primaryFlightControlWidget, tr("&Primary Controls"));

    SecondaryFlightControlWidget *secondaryFlightControlWidget = new SecondaryFlightControlWidget(this);
    ui->simulationVariablesTab->addTab(secondaryFlightControlWidget, tr("&Secondary Controls"));

    AircraftHandleWidget *aircraftHandleWidget = new AircraftHandleWidget(this);
    ui->simulationVariablesTab->addTab(aircraftHandleWidget, tr("&Handles && Brakes"));

    LightWidget *lightWidget = new LightWidget(this);
    ui->simulationVariablesTab->addTab(lightWidget, tr("&Lights"));
}

void SimulationVariablesDialog::updateUi() noexcept
{
    updateTitle();
}

// PRIVATE SLOTS

void SimulationVariablesDialog::updateTitle() noexcept
{
    QString windowTitle = QCoreApplication::translate("SimulationVariablesDialog", SimulationVariablesDialogPrivate::WindowTitle);
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const Connect::State state = skyConnect ? skyConnect->get().getState() : Connect::State::Disconnected;
    switch (state) {
    case Connect::State::Recording:
        windowTitle.append(" - " + tr("RECORDING"));
        break;
    case Connect::State::RecordingPaused:
        windowTitle.append(" - " + tr("RECORDING PAUSED"));
        break;
    case Connect::State::Replay:
        windowTitle.append(" - " + tr("PLAYBACK"));
        break;
    case Connect::State::ReplayPaused:
        windowTitle.append(" - " + tr("PLAYPACK PAUSED"));
        break;
    default:
        break;
    }
    setWindowTitle(windowTitle);
}
