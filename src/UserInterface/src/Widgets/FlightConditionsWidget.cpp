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
#include "../../../Model/src/AircraftInfo.h"
#include "../../../Model/src/FlightConditions.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "../../../Kernel/src/SkyMath.h"
#include "FlightConditionsWidget.h"
#include "ui_FlightConditionsWidget.h"

class FlightConditionsWidgetPrivate
{
public:
    FlightConditionsWidgetPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
};

// PUBLIC

FlightConditionsWidget::FlightConditionsWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QDialog(parent),
    d(std::make_unique<FlightConditionsWidgetPrivate>(skyConnect)),
    ui(std::make_unique<Ui::FlightConditionsWidget>())
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    initUi();
}

FlightConditionsWidget::~FlightConditionsWidget()
{
}

// PROTECTED

void FlightConditionsWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi();

    const Scenario &currentScenario = World::getInstance().getCurrentScenario();
    connect(&currentScenario, &Scenario::flightConditionsChanged,
            this, &FlightConditionsWidget::updateInfoUi);
}

void FlightConditionsWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const Scenario &currentScenario = World::getInstance().getCurrentScenario();
    disconnect(&currentScenario, &Scenario::flightConditionsChanged,
            this, &FlightConditionsWidget::updateInfoUi);
}

// PRIVATE

void FlightConditionsWidget::initUi()
{
    ui->groundAltitudeLineEdit->setToolTip(SimVar::GroundAltitude);
    ui->temperatureLineEdit->setToolTip(SimVar::AmbientTemperature);
    ui->totalAirTemperatureLineEdit->setToolTip(SimVar::TotalAirTemperature);
    ui->windVelocityLineEdit->setToolTip(SimVar::AmbientWindVelocity);
    ui->windDirectionLineEdit->setToolTip(SimVar::AmbientWindDirection);
    ui->precipitationStateLineEdit->setToolTip(SimVar::AmbientPrecipState);

    ui->inCloudsCheckBox->setToolTip(SimVar::AmbientInCloud);
    ui->visibilityLineEdit->setToolTip(SimVar::AmbientVisibility);
    ui->seaLevelPressure->setToolTip(SimVar::SeaLevelPressure);
    ui->pitotIcingLineEdit->setToolTip(SimVar::PitotIcePct);
    ui->structuralIcingLineEdit->setToolTip(SimVar::StructuralIcePct);

    // Make the flight information checkboxes checkable, but not for the user
    ui->inCloudsCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->inCloudsCheckBox->setFocusPolicy(Qt::NoFocus);
}

void FlightConditionsWidget::updateUi()
{
    updateInfoUi();
}

// PRIVATE SLOTS

void FlightConditionsWidget::updateInfoUi()
{
    const Scenario &currentScenario = World::getInstance().getCurrentScenario();
    const FlightConditions &flightConditions = currentScenario.getFlightConditionsConst();

    ui->groundAltitudeLineEdit->setText(QString::number(flightConditions.groundAltitude));
    ui->temperatureLineEdit->setText(QString::number(flightConditions.ambientTemperature));
    ui->totalAirTemperatureLineEdit->setText(QString::number(flightConditions.totalAirTemperature));
    ui->windVelocityLineEdit->setText(QString::number(flightConditions.windVelocity));
    ui->windDirectionLineEdit->setText(QString::number(flightConditions.windDirection));
    ui->precipitationStateLineEdit->setText(SimType::precipitationStateToString(flightConditions.precipitationState));

    ui->inCloudsCheckBox->setChecked(flightConditions.inClouds);
    ui->visibilityLineEdit->setText(QString::number(flightConditions.visibility));
    ui->seaLevelPressure->setText(QString::number(flightConditions.seaLevelPressure));
    ui->pitotIcingLineEdit->setText(QString::number(SkyMath::toPercent(flightConditions.pitotIcingPercent)));
    ui->structuralIcingLineEdit->setText(QString::number(SkyMath::toPercent(flightConditions.structuralIcingPercent)));
}
