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

#include <QWidget>

#include "../../../Kernel/src/Unit.h"
#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/FlightCondition.h"
#include "../../../Kernel/src/SkyMath.h"
#include "FlightConditionWidget.h"
#include "ui_FlightConditionWidget.h"

class FlightConditionWidgetPrivate
{
public:
    FlightConditionWidgetPrivate() noexcept
    {}

    Unit unit;
};

// PUBLIC

FlightConditionWidget::FlightConditionWidget(QWidget *parent) noexcept :
    QWidget(parent),
    d(std::make_unique<FlightConditionWidgetPrivate>()),
    ui(std::make_unique<Ui::FlightConditionWidget>())
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    initUi();
}

FlightConditionWidget::~FlightConditionWidget() noexcept
{}

// PROTECTED

void FlightConditionWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);
    updateUi();

    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::flightConditionChanged,
            this, &FlightConditionWidget::updateUi);
}

void FlightConditionWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::flightConditionChanged,
            this, &FlightConditionWidget::updateUi);
}

// PRIVATE

void FlightConditionWidget::initUi() noexcept
{
    ui->groundAltitudeLineEdit->setToolTip(SimVar::GroundAltitude);
    ui->surfaceTypeLineEdit->setToolTip(SimVar::SurfaceType);
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

// PRIVATE SLOTS

void FlightConditionWidget::updateUi() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const FlightCondition &flightCondition = flight.getFlightConditionConst();

    ui->groundAltitudeLineEdit->setText(d->unit.formatFeet(flightCondition.groundAltitude));
    ui->surfaceTypeLineEdit->setText(SimType::surfaceTypeToString(flightCondition.surfaceType));
    ui->temperatureLineEdit->setText(d->unit.formatCelcius(flightCondition.ambientTemperature));
    ui->totalAirTemperatureLineEdit->setText(d->unit.formatCelcius(flightCondition.totalAirTemperature));
    ui->windVelocityLineEdit->setText(d->unit.formatKnots(flightCondition.windVelocity));
    ui->windDirectionLineEdit->setText(d->unit.formatDegrees(flightCondition.windDirection));
    ui->precipitationStateLineEdit->setText(SimType::precipitationStateToString(flightCondition.precipitationState));

    ui->inCloudsCheckBox->setChecked(flightCondition.inClouds);
    ui->visibilityLineEdit->setText(d->unit.formatVisibility(flightCondition.visibility));
    ui->seaLevelPressure->setText(d->unit.formatPressureInHPa(flightCondition.seaLevelPressure));
    ui->pitotIcingLineEdit->setText(d->unit.formatPercent(SkyMath::toPercent(flightCondition.pitotIcingPercent)));
    ui->structuralIcingLineEdit->setText(d->unit.formatPercent(SkyMath::toPercent(flightCondition.structuralIcingPercent)));
    ui->startLocalSimulationTimeLineEdit->setText(d->unit.formatDateTime(flightCondition.startLocalTime));
    ui->endLocalSimulationTimeLineEdit->setText(d->unit.formatDateTime(flightCondition.endLocalTime));
    // Zulu time
    ui->startLocalSimulationTimeLineEdit->setToolTip(d->unit.formatDateTime(flightCondition.startZuluTime) + "Z");
    ui->endLocalSimulationTimeLineEdit->setToolTip(d->unit.formatDateTime(flightCondition.endZuluTime) + "Z");
}
