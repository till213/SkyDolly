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
#include <memory>

#include <QDialog>

#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/PrimaryFlightControl.h"
#include "../../../Model/src/AircraftInfo.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "../Widgets/AircraftWidget.h"
#include "../Widgets/EngineWidget.h"
#include "../Widgets/PrimaryFlightControlWidget.h"
#include "../Widgets/SecondaryFlightControlWidget.h"
#include "../Widgets/AircraftHandleWidget.h"
#include "../Widgets/LightWidget.h"
#include "SimulationVariablesDialog.h"
#include "ui_SimulationVariablesDialog.h"

class SimulationVariablesDialogPrivate
{
public:
    SimulationVariablesDialogPrivate(SkyConnectIntf &theSkyConnect) noexcept
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
    static const QString WindowTitle;
};

const QString SimulationVariablesDialogPrivate::WindowTitle = QT_TRANSLATE_NOOP("SimulationVariablesDialog", "Simulation Variables");

// PUBLIC

SimulationVariablesDialog::SimulationVariablesDialog(SkyConnectIntf &skyConnect, QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<SimulationVariablesDialogPrivate>(skyConnect)),
    ui(std::make_unique<Ui::SimulationVariablesDialog>())
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
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
    connect(&d->skyConnect, &SkyConnectIntf::stateChanged,
            this, &SimulationVariablesDialog::updateTitle);

    emit visibilityChanged(true);
}

void SimulationVariablesDialog::hideEvent(QHideEvent *event) noexcept
{
    QDialog::hideEvent(event);
    disconnect(&d->skyConnect, &SkyConnectIntf::stateChanged,
               this, &SimulationVariablesDialog::updateTitle);

    emit visibilityChanged(false);
}

// PRIVATE

void SimulationVariablesDialog::initUi() noexcept
{
    AircraftWidget *aircraftWidget = new AircraftWidget(d->skyConnect, this);
    ui->simulationVariablesTab->addTab(aircraftWidget, tr("&Aircraft"));

    EngineWidget *engineWidget = new EngineWidget(d->skyConnect, this);
    ui->simulationVariablesTab->addTab(engineWidget, tr("&Engine"));

    PrimaryFlightControlWidget *primaryFlightControlWidget = new PrimaryFlightControlWidget(d->skyConnect, this);
    ui->simulationVariablesTab->addTab(primaryFlightControlWidget, tr("&Primary Controls"));

    SecondaryFlightControlWidget *secondaryFlightControlWidget = new SecondaryFlightControlWidget(d->skyConnect, this);
    ui->simulationVariablesTab->addTab(secondaryFlightControlWidget, tr("&Secondary Controls"));

    AircraftHandleWidget *aircraftHandleWidget = new AircraftHandleWidget(d->skyConnect, this);
    ui->simulationVariablesTab->addTab(aircraftHandleWidget, tr("&Handles && Brakes"));

    LightWidget *lightWidget = new LightWidget(d->skyConnect, this);
    ui->simulationVariablesTab->addTab(lightWidget, tr("&Lights"));
}

void SimulationVariablesDialog::updateUi() noexcept
{
    updateTitle();
}

void SimulationVariablesDialog::updateTitle() noexcept
{
    QString windowTitle = SimulationVariablesDialogPrivate::WindowTitle;
    switch (d->skyConnect.getState()) {
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
