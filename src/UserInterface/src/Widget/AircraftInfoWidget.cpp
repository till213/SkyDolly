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

#include <QWidget>

#include <Kernel/Unit.h>
#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/AircraftType.h>
#include <Model/FlightCondition.h>
#include <Kernel/SkyMath.h>
#include "AircraftInfoWidget.h"
#include "ui_AircraftInfoWidget.h"

struct AircraftInfoWidgetPrivate
{
    Unit unit;
};

// PUBLIC

AircraftInfoWidget::AircraftInfoWidget(QWidget *parent) noexcept :
    QWidget {parent},
    ui {std::make_unique<Ui::AircraftInfoWidget>()},
    d {std::make_unique<AircraftInfoWidgetPrivate>()}
{
    ui->setupUi(this);
    initUi();
}

AircraftInfoWidget::~AircraftInfoWidget() = default;

// PROTECTED

void AircraftInfoWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);
    updateUi();

    const auto &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::aircraftInfoChanged,
            this, &AircraftInfoWidget::updateUi);
    connect(&flight, &Flight::userAircraftChanged,
            this, &AircraftInfoWidget::updateUi);
    connect(&flight, &Flight::flightStored,
            this, &AircraftInfoWidget::updateUi);
    connect(&flight, &Flight::flightRestored,
            this, &AircraftInfoWidget::updateUi);
    connect(&flight, &Flight::cleared,
            this, &AircraftInfoWidget::updateUi);
}

void AircraftInfoWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::aircraftInfoChanged,
               this, &AircraftInfoWidget::updateUi);
    disconnect(&flight, &Flight::userAircraftChanged,
               this, &AircraftInfoWidget::updateUi);
    disconnect(&flight, &Flight::flightStored,
               this, &AircraftInfoWidget::updateUi);
    disconnect(&flight, &Flight::flightRestored,
               this, &AircraftInfoWidget::updateUi);
    disconnect(&flight, &Flight::cleared,
               this, &AircraftInfoWidget::updateUi);
}

// PRIVATE

void AircraftInfoWidget::initUi() noexcept
{
    ui->nameLineEdit->setToolTip(SimVar::Title);
    ui->tailNumberLineEdit->setToolTip(SimVar::ATCFlightNumber);
    ui->airlineLineEdit->setToolTip(SimVar::ATCAirline);

    ui->categoryLineEdit->setToolTip(SimVar::Category);
    ui->startOnGroundCheckBox->setToolTip(SimVar::SimOnGround);
    ui->initialAirspeedLineEdit->setToolTip(SimVar::AirspeedTrue);
    ui->wingSpanLineEdit->setToolTip(SimVar::WingSpan);
    ui->engineTypeLineEdit->setToolTip(SimVar::EngineType);
    ui->numberOfEnginesLineEdit->setToolTip(SimVar::NumberOfEngines);
    ui->aircraftAltitudeAboveGroundLineEdit->setToolTip(SimVar::PlaneAltAboveGround);

    // Make the flight information checkboxes checkable, but not for the user
    ui->startOnGroundCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->startOnGroundCheckBox->setFocusPolicy(Qt::NoFocus);
}

// PRIVATE SLOTS

void AircraftInfoWidget::updateUi() noexcept
{
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    const auto &aircraft = flight.getUserAircraft();
    const auto &aircraftInfo = aircraft.getAircraftInfo();

    ui->nameLineEdit->setText(aircraftInfo.aircraftType.type);
    ui->tailNumberLineEdit->setText(aircraftInfo.tailNumber);
    ui->airlineLineEdit->setText(aircraftInfo.airline);
    ui->categoryLineEdit->setText(aircraftInfo.aircraftType.category);
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);

    ui->initialAirspeedLineEdit->setText(d->unit.formatKnots(aircraftInfo.initialAirspeed));
    ui->wingSpanLineEdit->setText(d->unit.formatFeet(aircraftInfo.aircraftType.wingSpan));
    ui->engineTypeLineEdit->setText(SimType::engineTypeToString(aircraftInfo.aircraftType.engineType));
    ui->numberOfEnginesLineEdit->setText(QString::number(aircraftInfo.aircraftType.numberOfEngines));
    ui->aircraftAltitudeAboveGroundLineEdit->setText(d->unit.formatFeet(aircraftInfo.altitudeAboveGround));
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);
}
