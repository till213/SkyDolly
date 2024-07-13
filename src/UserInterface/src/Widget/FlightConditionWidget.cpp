/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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

#include <QString>
#include <QStringBuilder>
#include <QWidget>

#include <Kernel/Const.h>
#include <Kernel/Unit.h>
#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/FlightCondition.h>
#include <Kernel/SkyMath.h>
#include "FlightConditionWidget.h"
#include "ui_FlightConditionWidget.h"

struct FlightConditionWidgetPrivate
{
    Unit unit;
};

// PUBLIC

FlightConditionWidget::FlightConditionWidget(QWidget *parent) noexcept :
    QWidget {parent},
    ui {std::make_unique<Ui::FlightConditionWidget>()},
    d {std::make_unique<FlightConditionWidgetPrivate>()}
{
    ui->setupUi(this);
    initUi();
}

FlightConditionWidget::~FlightConditionWidget() = default;

// PROTECTED

void FlightConditionWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);
    updateUi();

    // Flight
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::flightConditionChanged,
            this, &FlightConditionWidget::updateUi);
    connect(&flight, &Flight::flightStored,
            this, &FlightConditionWidget::updateUi);
    connect(&flight, &Flight::flightRestored,
            this, &FlightConditionWidget::updateUi);
    connect(&flight, &Flight::cleared,
            this, &FlightConditionWidget::updateUi);
}

void FlightConditionWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);

    // Flight
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::flightConditionChanged,
               this, &FlightConditionWidget::updateUi);
    disconnect(&flight, &Flight::flightStored,
               this, &FlightConditionWidget::updateUi);
    disconnect(&flight, &Flight::flightRestored,
               this, &FlightConditionWidget::updateUi);
    disconnect(&flight, &Flight::cleared,
               this, &FlightConditionWidget::updateUi);
}

// PRIVATE

void FlightConditionWidget::initUi() noexcept
{
    ui->groundAltitudeLineEdit->setToolTip(SimVar::GroundAltitude);
    ui->surfaceTypeLineEdit->setToolTip(SimVar::SurfaceType);
    ui->surfaceConditionLineEdit->setToolTip(SimVar::SurfaceCondition);
    ui->temperatureLineEdit->setToolTip(SimVar::AmbientTemperature);
    ui->totalAirTemperatureLineEdit->setToolTip(SimVar::TotalAirTemperature);
    ui->windSpeedLineEdit->setToolTip(SimVar::AmbientWindVelocity);
    ui->windDirectionLineEdit->setToolTip(SimVar::AmbientWindDirection);
    ui->precipitationStateLineEdit->setToolTip(SimVar::AmbientPrecipState);

    ui->onAnyRunwayCheckBox->setToolTip(SimVar::OnAnyRunway);
    ui->onParkingSpotCheckBox->setToolTip(SimVar::AtcOnParkingSpot);
    ui->inCloudsCheckBox->setToolTip(SimVar::AmbientInCloud);
    ui->visibilityLineEdit->setToolTip(SimVar::AmbientVisibility);
    ui->seaLevelPressure->setToolTip(SimVar::SeaLevelPressure);
    ui->pitotIcingLineEdit->setToolTip(SimVar::PitotIcePct);
    ui->structuralIcingLineEdit->setToolTip(SimVar::StructuralIcePct);

    // Make the flight information checkboxes checkable, but not for the user
    ui->inCloudsCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->inCloudsCheckBox->setFocusPolicy(Qt::NoFocus);
}

// PRIVATE SLOTS

void FlightConditionWidget::updateUi() noexcept
{
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    const auto &flightCondition = flight.getFlightCondition();

    ui->groundAltitudeLineEdit->setText(d->unit.formatFeet(flightCondition.groundAltitude));
    ui->surfaceTypeLineEdit->setText(SimType::surfaceTypeToString(flightCondition.surfaceType));
    ui->surfaceConditionLineEdit->setText(SimType::surfaceConditionToString(flightCondition.surfaceCondition));
    ui->temperatureLineEdit->setText(d->unit.formatCelcius(flightCondition.ambientTemperature));
    ui->totalAirTemperatureLineEdit->setText(d->unit.formatCelcius(flightCondition.totalAirTemperature));
    ui->windSpeedLineEdit->setText(d->unit.formatKnots(flightCondition.windSpeed));
    ui->windDirectionLineEdit->setText(d->unit.formatDegrees(flightCondition.windDirection));
    ui->precipitationStateLineEdit->setText(SimType::precipitationStateToString(flightCondition.precipitationState));

    ui->onAnyRunwayCheckBox->setChecked(flightCondition.onAnyRunway);
    ui->onParkingSpotCheckBox->setChecked(flightCondition.onParkingSpot);
    ui->inCloudsCheckBox->setChecked(flightCondition.inClouds);
    ui->visibilityLineEdit->setText(d->unit.formatVisibility(flightCondition.visibility));
    ui->seaLevelPressure->setText(d->unit.formatPressureInHPa(flightCondition.seaLevelPressure));
    ui->pitotIcingLineEdit->setText(d->unit.formatPercent(flightCondition.pitotIcingPercent));
    ui->structuralIcingLineEdit->setText(d->unit.formatPercent(flightCondition.structuralIcingPercent));
    ui->startLocalSimulationTimeLineEdit->setText(d->unit.formatDateTime(flightCondition.startLocalDateTime));
    ui->endLocalSimulationTimeLineEdit->setText(d->unit.formatDateTime(flightCondition.endLocalDateTime));
    // Zulu time
    ui->startLocalSimulationTimeLineEdit->setToolTip(d->unit.formatDateTime(flightCondition.getStartZuluDateTime()) % Const::ZuluTimeSuffix);
    ui->endLocalSimulationTimeLineEdit->setToolTip(d->unit.formatDateTime(flightCondition.getEndZuluDateTime()) % Const::ZuluTimeSuffix);
}
