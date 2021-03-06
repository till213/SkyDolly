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
#include "../../../Model/src/AircraftInfo.h"
#include "../../../Model/src/AircraftType.h"
#include "../../../Model/src/FlightCondition.h"
#include "../../../Kernel/src/SkyMath.h"
#include "AircraftTypeWidget.h"
#include "ui_AircraftTypeWidget.h"

class AircraftTypeWidgetPrivate
{
public:
    AircraftTypeWidgetPrivate() noexcept
    {}

    Unit unit;
};

// PUBLIC

AircraftTypeWidget::AircraftTypeWidget(QWidget *parent) noexcept :
    QWidget(parent),
    d(std::make_unique<AircraftTypeWidgetPrivate>()),
    ui(std::make_unique<Ui::AircraftTypeWidget>())
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    initUi();
}

AircraftTypeWidget::~AircraftTypeWidget() noexcept
{}

// PROTECTED

void AircraftTypeWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);
    updateUi();

    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const Aircraft &aircraft = flight.getUserAircraft();
    connect(&aircraft, &Aircraft::infoChanged,
            this, &AircraftTypeWidget::updateUi);
    connect(&flight, &Flight::userAircraftChanged,
            this, &AircraftTypeWidget::updateUi);
}

void AircraftTypeWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const Aircraft &aircraft = flight.getUserAircraft();
    disconnect(&aircraft, &Aircraft::infoChanged,
            this, &AircraftTypeWidget::updateUi);
}

// PRIVATE

void AircraftTypeWidget::initUi() noexcept
{
    ui->nameLineEdit->setToolTip(SimVar::Title);
    ui->tailNumberLineEdit->setToolTip(SimVar::ATCFlightNumber);
    ui->airlineLineEdit->setToolTip(SimVar::ATCAirline);
    ui->flightLineEdit->setToolTip(SimVar::ATCId);

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

void AircraftTypeWidget::updateUi() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const Aircraft &aircraft = flight.getUserAircraftConst();
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfoConst();

    ui->nameLineEdit->setText(aircraftInfo.aircraftType.type);
    ui->tailNumberLineEdit->setText(aircraftInfo.tailNumber);
    ui->airlineLineEdit->setText(aircraftInfo.airline);
    ui->flightLineEdit->setText(aircraftInfo.flightNumber);
    ui->categoryLineEdit->setText(aircraftInfo.aircraftType.category);
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);

    ui->initialAirspeedLineEdit->setText(d->unit.formatKnots(aircraftInfo.initialAirspeed));
    ui->wingSpanLineEdit->setText(d->unit.formatFeet(aircraftInfo.aircraftType.wingSpan));
    ui->engineTypeLineEdit->setText(SimType::engineTypeToString(aircraftInfo.aircraftType.engineType));
    ui->numberOfEnginesLineEdit->setText(QString::number(aircraftInfo.aircraftType.numberOfEngines));
    ui->aircraftAltitudeAboveGroundLineEdit->setText(d->unit.formatFeet(aircraftInfo.altitudeAboveGround));
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);
}
