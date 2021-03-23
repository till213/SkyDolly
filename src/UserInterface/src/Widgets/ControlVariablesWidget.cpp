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
#include "../../../Model/src/World.h"
#include "../../../Model/src/Scenario.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/AircraftData.h"
#include "../../../Model/src/EngineData.h"
#include "../../../Model/src/AircraftInfo.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "ControlVariablesWidget.h"
#include "ui_ControlVariablesWidget.h"

class ControlVariablesWidgetPrivate
{
public:
    ControlVariablesWidgetPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
};

ControlVariablesWidget::ControlVariablesWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QWidget(parent),
    d(std::make_unique<ControlVariablesWidgetPrivate>(skyConnect)),
    ui(std::make_unique<Ui::ControlVariablesWidget>())
{
    ui->setupUi(this);
    initUi();
}

ControlVariablesWidget::~ControlVariablesWidget()
{
}

// PROTECTED

void ControlVariablesWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi();

    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();
    // Signal sent while recording
    connect(&aircraft, &Aircraft::dataChanged,
            this, &ControlVariablesWidget::updateControlDataUi);
    // Signal sent while playing
    connect(&d->skyConnect, &SkyConnectIntf::currentTimestampChanged,
            this, &ControlVariablesWidget::updateControlDataUi);
}

void ControlVariablesWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();
    disconnect(&aircraft, &Aircraft::dataChanged,
               this, &ControlVariablesWidget::updateControlDataUi);
    disconnect(&d->skyConnect, &SkyConnectIntf::currentTimestampChanged,
            this, &ControlVariablesWidget::updateControlDataUi);
}

// PRIVATE

void ControlVariablesWidget::initUi()
{
    ui->yokeXLineEdit->setToolTip(SimVar::YokeXPosition);
    ui->yokeYLineEdit->setToolTip(SimVar::YokeYPosition);
    ui->rudderLineEdit->setToolTip(SimVar::RudderPosition);
    ui->elevatorLineEdit->setToolTip(SimVar::ElevatorPosition);
    ui->aileronLineEdit->setToolTip(SimVar::AileronPosition);

    ui->throttle1LineEdit->setToolTip(SimVar::ThrottleLeverPosition1);
    ui->throttle2LineEdit->setToolTip(SimVar::ThrottleLeverPosition2);
    ui->throttle3LineEdit->setToolTip(SimVar::ThrottleLeverPosition3);
    ui->throttle4LineEdit->setToolTip(SimVar::ThrottleLeverPosition4);
    ui->propeller1LineEdit->setToolTip(SimVar::PropellerLeverPosition1);
    ui->propeller2LineEdit->setToolTip(SimVar::PropellerLeverPosition2);
    ui->propeller3LineEdit->setToolTip(SimVar::PropellerLeverPosition3);
    ui->propeller4LineEdit->setToolTip(SimVar::PropellerLeverPosition4);
    ui->mixture1LineEdit->setToolTip(SimVar::MixtureLeverPosition1);
    ui->mixture2LineEdit->setToolTip(SimVar::MixtureLeverPosition2);
    ui->mixture3LineEdit->setToolTip(SimVar::MixtureLeverPosition3);
    ui->mixture4LineEdit->setToolTip(SimVar::MixtureLeverPosition4);

    ui->leadingEdgeFlapsLeftLineEdit->setToolTip(SimVar::LeadingEdgeFlapsLeftPercent);
    ui->leadingEdgeFlapsRightLineEdit->setToolTip(SimVar::LeadingEdgeFlapsRightPercent);
    ui->trailingEdgeFlapsLeftLineEdit->setToolTip(SimVar::TrailingEdgeFlapsLeftPercent);
    ui->trailingEdgeFlapsRightLineEdit->setToolTip(SimVar::TrailingEdgeFlapsRightPercent);
    ui->flapsPositionLineEdit->setToolTip(SimVar::FlapsHandleIndex);
    ui->spoilerLineEdit->setToolTip(SimVar::SpoilersHandlePosition);

    ui->gearLineEdit->setToolTip(SimVar::GearHandlePosition);
    ui->brakeLeftLineEdit->setToolTip(SimVar::BrakeLeftPosition);
    ui->brakeRightLineEdit->setToolTip(SimVar::BrakeRightPosition);
    ui->waterRudderLineEdit->setToolTip(SimVar::WaterRudderHandlePosition);
    ui->tailhookLineEdit->setToolTip(SimVar::TailhookPosition);
    ui->canopyOpenLineEdit->setToolTip(SimVar::CanopyOpen);
}

void ControlVariablesWidget::updateUi()
{
    updateControlDataUi();
}

const AircraftData &ControlVariablesWidget::getCurrentAircraftData() const
{
    const AircraftData aircraftData;
    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();

    if (d->skyConnect.getState() == Connect::State::Recording) {
        return aircraft.getLastAircraftData();
    } else {
        return d->skyConnect.getCurrentAircraftData();
    };
}

const EngineData &ControlVariablesWidget::getCurrentEngineData() const
{
    const AircraftData aircraftData;
    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();

    if (d->skyConnect.getState() == Connect::State::Recording) {
        return aircraft.getEngineConst().getLastEngineData();
    } else {
        return aircraft.getEngineConst().interpolateEngineData(d->skyConnect.getCurrentTimestamp());
    };
}

// PRIVATE SLOTS

void ControlVariablesWidget::updateControlDataUi()
{
    const AircraftData &aircraftData = getCurrentAircraftData();
    const EngineData &engineData = getCurrentEngineData();

    // Aircraft controls
    ui->yokeXLineEdit->setText(QString::number(aircraftData.yokeXPosition));
    ui->yokeYLineEdit->setText(QString::number(aircraftData.yokeYPosition));
    ui->rudderLineEdit->setText(QString::number(aircraftData.rudderPosition));
    ui->elevatorLineEdit->setText(QString::number(aircraftData.elevatorPosition));
    ui->aileronLineEdit->setText(QString::number(aircraftData.aileronPosition));

    // General engine
    ui->throttle1LineEdit->setText(QString::number(engineData.throttleLeverPosition1));
    ui->throttle2LineEdit->setText(QString::number(engineData.throttleLeverPosition2));
    ui->throttle3LineEdit->setText(QString::number(engineData.throttleLeverPosition3));
    ui->throttle4LineEdit->setText(QString::number(engineData.throttleLeverPosition4));
    ui->propeller1LineEdit->setText(QString::number(engineData.propellerLeverPosition1));
    ui->propeller2LineEdit->setText(QString::number(engineData.propellerLeverPosition2));
    ui->propeller3LineEdit->setText(QString::number(engineData.propellerLeverPosition3));
    ui->propeller4LineEdit->setText(QString::number(engineData.propellerLeverPosition4));
    ui->mixture1LineEdit->setText(QString::number(engineData.mixtureLeverPosition1));
    ui->mixture2LineEdit->setText(QString::number(engineData.mixtureLeverPosition2));
    ui->mixture3LineEdit->setText(QString::number(engineData.mixtureLeverPosition3));
    ui->mixture4LineEdit->setText(QString::number(engineData.mixtureLeverPosition4));

    // Flaps & speed brakes
    ui->leadingEdgeFlapsLeftLineEdit->setText(QString::number(aircraftData.leadingEdgeFlapsLeftPercent));
    ui->leadingEdgeFlapsRightLineEdit->setText(QString::number(aircraftData.leadingEdgeFlapsRightPercent));
    ui->trailingEdgeFlapsLeftLineEdit->setText(QString::number(aircraftData.trailingEdgeFlapsLeftPercent));
    ui->trailingEdgeFlapsRightLineEdit->setText(QString::number(aircraftData.trailingEdgeFlapsRightPercent));
    ui->spoilerLineEdit->setText(QString::number(aircraftData.spoilersHandlePosition));
    ui->flapsPositionLineEdit->setText(QString::number(aircraftData.flapsHandleIndex));

    // // Gear, brakes & handles
    aircraftData.gearHandlePosition ? ui->gearLineEdit->setText(tr("Down")) : ui->gearLineEdit->setText(tr("Up"));
    ui->brakeLeftLineEdit->setText(QString::number(aircraftData.brakeLeftPosition));
    ui->brakeRightLineEdit->setText(QString::number(aircraftData.brakeRightPosition));
    ui->waterRudderLineEdit->setText(QString::number(aircraftData.waterRudderHandlePosition));
    ui->tailhookLineEdit->setText(QString::number(aircraftData.tailhookPosition));
    ui->canopyOpenLineEdit->setText(QString::number(aircraftData.canopyOpen));
}

