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

#include "../../../Kernel/src/Const.h"
#include "../../../Kernel/src/Aircraft.h"
#include "../../../Kernel/src/AircraftInfo.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "SimulationVariablesDialog.h"
#include "ui_SimulationVariablesDialog.h"

class SimulationVariablesDialogPrivate
{
public:
    SimulationVariablesDialogPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
    static const QString WindowTitle;
};

const QString SimulationVariablesDialogPrivate::WindowTitle = QT_TRANSLATE_NOOP("SimulationVariablesDialog", "Simulation Variables");

// PUBLIC

SimulationVariablesDialog::SimulationVariablesDialog(SkyConnectIntf &skyConnect, QWidget *parent) :
    QDialog(parent),
    d(std::make_unique<SimulationVariablesDialogPrivate>(skyConnect)),
    ui(new Ui::SimulationVariablesDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    initUi();
}

SimulationVariablesDialog::~SimulationVariablesDialog()
{
    delete ui;
}

// PROTECTED

void SimulationVariablesDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi();

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    // Signal sent while recording
    connect(&aircraft, &Aircraft::dataChanged,
            this, &SimulationVariablesDialog::updateAircraftDataUi);
    connect(&aircraft, &Aircraft::infoChanged,
            this, &SimulationVariablesDialog::updateInfoUi);
    // Signal sent while playing
    connect(&d->skyConnect, &SkyConnectIntf::aircraftDataSent,
            this, &SimulationVariablesDialog::updateAircraftDataUi);

    connect(&d->skyConnect, &SkyConnectIntf::stateChanged,
            this, &SimulationVariablesDialog::updateTitle);

    emit visibilityChanged(true);
}

void SimulationVariablesDialog::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    disconnect(&aircraft, &Aircraft::dataChanged,
               this, &SimulationVariablesDialog::updateAircraftDataUi);
    disconnect(&aircraft, &Aircraft::infoChanged,
            this, &SimulationVariablesDialog::updateInfoUi);
    disconnect(&d->skyConnect, &SkyConnectIntf::aircraftDataSent,
            this, &SimulationVariablesDialog::updateAircraftDataUi);
    disconnect(&d->skyConnect, &SkyConnectIntf::stateChanged,
               this, &SimulationVariablesDialog::updateTitle);

    emit visibilityChanged(false);
}

// PRIVATE

void SimulationVariablesDialog::initUi()
{
    ui->latitudeLineEdit->setToolTip(Const::Latitude);
    ui->longitudeLineEdit->setToolTip(Const::Longitude);
    ui->altitudeLineEdit->setToolTip(Const::Altitude);
    ui->pitchLineEdit->setToolTip(Const::Pitch);
    ui->bankLineEdit->setToolTip(Const::Bank);
    ui->headingLineEdit->setToolTip(Const::Heading);

    ui->yokeXLineEdit->setToolTip(Const::YokeXPosition);
    ui->yokeYLineEdit->setToolTip(Const::YokeYPosition);
    ui->rudderLineEdit->setToolTip(Const::RudderPosition);
    ui->elevatorLineEdit->setToolTip(Const::ElevatorPosition);
    ui->aileronLineEdit->setToolTip(Const::AileronPosition);

    ui->throttle1LineEdit->setToolTip(Const::ThrottleLeverPosition1);
    ui->throttle2LineEdit->setToolTip(Const::ThrottleLeverPosition2);
    ui->throttle3LineEdit->setToolTip(Const::ThrottleLeverPosition3);
    ui->throttle4LineEdit->setToolTip(Const::ThrottleLeverPosition4);
    ui->propeller1LineEdit->setToolTip(Const::PropellerLeverPosition1);
    ui->propeller2LineEdit->setToolTip(Const::PropellerLeverPosition2);
    ui->propeller3LineEdit->setToolTip(Const::PropellerLeverPosition3);
    ui->propeller4LineEdit->setToolTip(Const::PropellerLeverPosition4);
    ui->mixture1LineEdit->setToolTip(Const::MixtureLeverPosition1);
    ui->mixture2LineEdit->setToolTip(Const::MixtureLeverPosition2);
    ui->mixture3LineEdit->setToolTip(Const::MixtureLeverPosition3);
    ui->mixture4LineEdit->setToolTip(Const::MixtureLeverPosition4);

    ui->leadingEdgeFlapsLeftLineEdit->setToolTip(Const::LeadingEdgeFlapsLeftPercent);
    ui->leadingEdgeFlapsRightLineEdit->setToolTip(Const::LeadingEdgeFlapsRightPercent);
    ui->trailingEdgeFlapsLeftLineEdit->setToolTip(Const::TrailingEdgeFlapsLeftPercent);
    ui->trailingEdgeFlapsRightLineEdit->setToolTip(Const::TrailingEdgeFlapsRightPercent);
    ui->flapsPositionLineEdit->setToolTip(Const::FlapsHandleIndex);
    ui->spoilerLineEdit->setToolTip(Const::SpoilersHandlePosition);

    ui->gearLineEdit->setToolTip(Const::GearHandlePosition);
    ui->waterRudderLineEdit->setToolTip(Const::WaterRudderHandlePosition);
    ui->brakeLeftLineEdit->setToolTip(Const::BrakeLeftPosition);
    ui->brakeRightLineEdit->setToolTip(Const::BrakeRightPosition);

    // Make the "on ground" checkbox checkable, but not for the user
    ui->startOnGroundCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->startOnGroundCheckBox->setFocusPolicy(Qt::NoFocus);
}

void SimulationVariablesDialog::updateUi()
{
    updateTitle();
    updateInfoUi();
    updateAircraftDataUi();
}

const AircraftData &SimulationVariablesDialog::getCurrentAircraftData() const
{
    const AircraftData aircraftData;
    const Aircraft &aircraft = d->skyConnect.getAircraft();

    if (d->skyConnect.getState() == Connect::State::Recording) {
        return aircraft.getLastAircraftData();
    } else {
        return d->skyConnect.getCurrentAircraftData();
    };
}

// PRIVATE SLOTS

void SimulationVariablesDialog::updateInfoUi()
{
    const Aircraft &aircraft = d->skyConnect.getAircraft();
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();

    ui->nameLineEdit->setText(aircraftInfo.name);
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);
    ui->initialAirspeedLineEdit->setText(QString::number(aircraftInfo.initialAirspeed));
}

void SimulationVariablesDialog::updateAircraftDataUi()
{
    const AircraftData &aircraftData = getCurrentAircraftData();

    // Aircraft position
    ui->latitudeLineEdit->setText(QString::number(aircraftData.latitude));
    ui->longitudeLineEdit->setText(QString::number(aircraftData.longitude));
    ui->altitudeLineEdit->setText(QString::number(aircraftData.altitude));
    ui->pitchLineEdit->setText(QString::number(aircraftData.pitch));
    ui->bankLineEdit->setText(QString::number(aircraftData.bank));
    ui->headingLineEdit->setText(QString::number(aircraftData.heading));

    // Aircraft controls
    ui->yokeXLineEdit->setText(QString::number(aircraftData.yokeXPosition));
    ui->yokeYLineEdit->setText(QString::number(aircraftData.yokeYPosition));
    ui->rudderLineEdit->setText(QString::number(aircraftData.rudderPosition));
    ui->elevatorLineEdit->setText(QString::number(aircraftData.elevatorPosition));
    ui->aileronLineEdit->setText(QString::number(aircraftData.aileronPosition));

    // General engine
    ui->throttle1LineEdit->setText(QString::number(aircraftData.throttleLeverPosition1));
    ui->throttle2LineEdit->setText(QString::number(aircraftData.throttleLeverPosition2));
    ui->throttle3LineEdit->setText(QString::number(aircraftData.throttleLeverPosition3));
    ui->throttle4LineEdit->setText(QString::number(aircraftData.throttleLeverPosition4));
    ui->propeller1LineEdit->setText(QString::number(aircraftData.propellerLeverPosition1));
    ui->propeller2LineEdit->setText(QString::number(aircraftData.propellerLeverPosition2));
    ui->propeller3LineEdit->setText(QString::number(aircraftData.propellerLeverPosition3));
    ui->propeller4LineEdit->setText(QString::number(aircraftData.propellerLeverPosition4));
    ui->mixture1LineEdit->setText(QString::number(aircraftData.mixtureLeverPosition1));
    ui->mixture2LineEdit->setText(QString::number(aircraftData.mixtureLeverPosition2));
    ui->mixture3LineEdit->setText(QString::number(aircraftData.mixtureLeverPosition3));
    ui->mixture4LineEdit->setText(QString::number(aircraftData.mixtureLeverPosition4));

    // Flaps & speed brakes
    ui->leadingEdgeFlapsLeftLineEdit->setText(QString::number(aircraftData.leadingEdgeFlapsLeftPercent));
    ui->leadingEdgeFlapsRightLineEdit->setText(QString::number(aircraftData.leadingEdgeFlapsRightPercent));
    ui->trailingEdgeFlapsLeftLineEdit->setText(QString::number(aircraftData.trailingEdgeFlapsLeftPercent));
    ui->trailingEdgeFlapsRightLineEdit->setText(QString::number(aircraftData.trailingEdgeFlapsRightPercent));
    ui->spoilerLineEdit->setText(QString::number(aircraftData.spoilersHandlePosition));
    ui->flapsPositionLineEdit->setText(QString::number(aircraftData.flapsHandleIndex));

    // Gear & brakes
    aircraftData.gearHandlePosition ? ui->gearLineEdit->setText(tr("Down")) : ui->gearLineEdit->setText(tr("Up"));
    ui->waterRudderLineEdit->setText(QString::number(aircraftData.waterRudderHandlePosition));
    ui->brakeLeftLineEdit->setText(QString::number(aircraftData.brakeLeftPosition));
    ui->brakeRightLineEdit->setText(QString::number(aircraftData.brakeRightPosition));
}

void SimulationVariablesDialog::updateTitle()
{
    QString windowTitle = SimulationVariablesDialogPrivate::WindowTitle;
    switch (d->skyConnect.getState()) {
    case Connect::State::Recording:
        windowTitle.append(" - " + tr("RECORDING"));
        break;
    case Connect::State::RecordingPaused:
        windowTitle.append(" - " + tr("RECORDING PAUSED"));
        break;
    case Connect::State::Playback:
        windowTitle.append(" - " + tr("PLAYBACK"));
        break;
    case Connect::State::PlaybackPaused:
        windowTitle.append(" - " + tr("PLAYPACK PAUSED"));
        break;
    default:
        break;
    }
    setWindowTitle(windowTitle);
}
