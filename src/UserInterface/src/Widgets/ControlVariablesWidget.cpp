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
        return aircraft.interpolateAircraftData(d->skyConnect.getCurrentTimestamp());
    };
}

// PRIVATE SLOTS

void ControlVariablesWidget::updateControlDataUi()
{
    const AircraftData &aircraftData = getCurrentAircraftData();

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
