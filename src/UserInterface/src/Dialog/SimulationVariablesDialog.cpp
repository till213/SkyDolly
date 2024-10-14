/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <optional>

#include <QCoreApplication>
#include <QString>
#include <QStringBuilder>
#include <QWidget>
#include <QDialog>
#include <QShortcut>
#include <QShowEvent>
#include <QHideEvent>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Aircraft.h>
#include <Model/PrimaryFlightControl.h>
#include <Model/AircraftInfo.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Connect/Connect.h>
#include "../Widget/AircraftWidget.h"
#include "../Widget/EngineWidget.h"
#include "../Widget/PrimaryFlightControlWidget.h"
#include "../Widget/SecondaryFlightControlWidget.h"
#include "../Widget/AircraftHandleWidget.h"
#include "../Widget/LightWidget.h"
#include "SimulationVariablesDialog.h"
#include "ui_SimulationVariablesDialog.h"

struct SimulationVariablesDialogPrivate
{
    QShortcut *closeDialogShortcut {nullptr};
};

// PUBLIC

SimulationVariablesDialog::SimulationVariablesDialog(QWidget *parent) noexcept :
    QDialog {parent},
    ui {std::make_unique<Ui::SimulationVariablesDialog>()},
    d {std::make_unique<SimulationVariablesDialogPrivate>()}
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

SimulationVariablesDialog::~SimulationVariablesDialog() = default;

// PROTECTED

void SimulationVariablesDialog::showEvent(QShowEvent *event) noexcept
{
    QDialog::showEvent(event);
    updateUi();
    auto &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &SimulationVariablesDialog::updateTitle);

    emit visibilityChanged(true);
}

void SimulationVariablesDialog::hideEvent(QHideEvent *event) noexcept
{
    QDialog::hideEvent(event);
    auto &skyConnectManager = SkyConnectManager::getInstance();
    disconnect(&skyConnectManager, &SkyConnectManager::stateChanged,
               this, &SimulationVariablesDialog::updateTitle);

    emit visibilityChanged(false);
}

// PRIVATE

/// \todo DRY: "centrally" define the "V" shortcut (currently also assigned to the corresponding QAction)
void SimulationVariablesDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    std::unique_ptr<AircraftWidget> aircraftWidget = std::make_unique<AircraftWidget>(this);
    // Transfer ownership to Qt
    ui->simulationVariablesTab->addTab(aircraftWidget.release(), tr("&Aircraft"));

    std::unique_ptr<EngineWidget> engineWidget = std::make_unique<EngineWidget>(this);
    ui->simulationVariablesTab->addTab(engineWidget.release(), tr("&Engine"));

    std::unique_ptr<PrimaryFlightControlWidget> primaryFlightControlWidget = std::make_unique<PrimaryFlightControlWidget>(this);
    ui->simulationVariablesTab->addTab(primaryFlightControlWidget.release(), tr("&Primary Controls"));

    std::unique_ptr<SecondaryFlightControlWidget> secondaryFlightControlWidget = std::make_unique<SecondaryFlightControlWidget>(this);
    ui->simulationVariablesTab->addTab(secondaryFlightControlWidget.release(), tr("&Secondary Controls"));

    std::unique_ptr<AircraftHandleWidget> aircraftHandleWidget = std::make_unique<AircraftHandleWidget>(this);
    ui->simulationVariablesTab->addTab(aircraftHandleWidget.release(), tr("&Handles && Brakes"));

    std::unique_ptr<LightWidget> lightWidget = std::make_unique<LightWidget>(this);
    ui->simulationVariablesTab->addTab(lightWidget.release(), tr("&Lights"));

    d->closeDialogShortcut = new QShortcut(QKeySequence(tr("V", "Window|Simulation Variables...")), this);
}

void SimulationVariablesDialog::updateUi() noexcept
{
    updateTitle();
}

void SimulationVariablesDialog::frenchConnection() noexcept
{
    connect(d->closeDialogShortcut, &QShortcut::activated,
            this, &SimulationVariablesDialog::close);
}

// PRIVATE SLOTS

void SimulationVariablesDialog::updateTitle() noexcept
{
    QString windowTitle = tr("Simulation Variables");
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const Connect::State state = skyConnect ? skyConnect->get().getState() : Connect::State::Disconnected;
    switch (state) {
    case Connect::State::Disconnected:
        windowTitle.append(" - " % tr("DISCONNECTED"));
        break;
    case Connect::State::Connected:
        windowTitle.append(" - " % tr("CONNECTED"));
        break;
    case Connect::State::Recording:
        windowTitle.append(" - " % tr("RECORDING"));
        break;
    case Connect::State::RecordingPaused:
        windowTitle.append(" - " % tr("RECORDING PAUSED"));
        break;
    case Connect::State::Replay:
        windowTitle.append(" - " % tr("PLAYBACK"));
        break;
    case Connect::State::ReplayPaused:
        windowTitle.append(" - " % tr("PLAYPACK PAUSED"));
        break;
    }
    setWindowTitle(windowTitle);
}
