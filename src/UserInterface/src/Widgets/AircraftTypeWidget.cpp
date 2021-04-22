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
#include "../../../Model/src/FlightCondition.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "../../../Kernel/src/SkyMath.h"
#include "../Unit.h"
#include "AircraftTypeWidget.h"
#include "ui_AircraftTypeWidget.h"

class AircraftTypeWidgetPrivate
{
public:
    AircraftTypeWidgetPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
    Unit unit;
};

// PUBLIC

AircraftTypeWidget::AircraftTypeWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QDialog(parent),
    d(std::make_unique<AircraftTypeWidgetPrivate>(skyConnect)),
    ui(std::make_unique<Ui::AircraftTypeWidget>())
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    initUi();
}

AircraftTypeWidget::~AircraftTypeWidget()
{
}

// PROTECTED

void AircraftTypeWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi();

    const Scenario &currentScenario = World::getInstance().getCurrentScenario();
    const Aircraft &aircraft = currentScenario.getUserAircraft();
    connect(&aircraft, &Aircraft::infoChanged,
            this, &AircraftTypeWidget::updateInfoUi);
}

void AircraftTypeWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const Scenario &currentScenario = World::getInstance().getCurrentScenario();
    const Aircraft &aircraft = currentScenario.getUserAircraft();
    disconnect(&aircraft, &Aircraft::infoChanged,
            this, &AircraftTypeWidget::updateInfoUi);
}

// PRIVATE

void AircraftTypeWidget::initUi()
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

void AircraftTypeWidget::updateUi()
{
    updateInfoUi();
}

// PRIVATE SLOTS

void AircraftTypeWidget::updateInfoUi()
{
    const Scenario &currentScenario = World::getInstance().getCurrentScenario();
    const Aircraft &aircraft = currentScenario.getUserAircraftConst();
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();

    ui->nameLineEdit->setText(aircraftInfo.name);
    ui->tailNumberLineEdit->setText(aircraftInfo.atcId);
    ui->airlineLineEdit->setText(aircraftInfo.atcAirline);
    ui->flightLineEdit->setText(aircraftInfo.atcFlightNumber);
    ui->categoryLineEdit->setText(aircraftInfo.category);
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);

    ui->initialAirspeedLineEdit->setText(d->unit.formatKnots(aircraftInfo.initialAirspeed));
    ui->wingSpanLineEdit->setText(d->unit.formatFeet(aircraftInfo.wingSpan));
    ui->engineTypeLineEdit->setText(SimType::engineTypeToString(aircraftInfo.engineType));
    ui->numberOfEnginesLineEdit->setText(QString::number(aircraftInfo.numberOfEngines));
    ui->aircraftAltitudeAboveGroundLineEdit->setText(d->unit.formatFeet(aircraftInfo.aircraftAltitudeAboveGround));
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);
}
