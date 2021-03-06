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
#include "FlightDialog.h"
#include "ui_FlightDialog.h"

#include <memory>

#include <QDialog>

#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/PrimaryFlightControl.h"
#include "../../../Model/src/AircraftInfo.h"
#include "../../../Persistence/src/Service/FlightService.h"
#include "../Widgets/FlightDescriptionWidget.h"
#include "../Widgets/AircraftTypeWidget.h"
#include "../Widgets/FlightConditionWidget.h"
#include "../Widgets/FlightPlanWidget.h"
#include "FlightDialog.h"
#include "ui_FlightDialog.h"

class FlightDialogPrivate
{
public:
    FlightDialogPrivate(FlightService &theFlightService) noexcept
        : flightService(theFlightService)
    {}

    FlightService &flightService;
};

// PUBLIC

FlightDialog::FlightDialog(FlightService &flightService, QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<FlightDialogPrivate>(flightService)),
    ui(std::make_unique<Ui::FlightDialog>())
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
    initUi();
}

FlightDialog::~FlightDialog() noexcept
{}

// PROTECTED

void FlightDialog::showEvent(QShowEvent *event) noexcept
{
    QDialog::showEvent(event);
    updateUi();
    emit visibilityChanged(true);
}

void FlightDialog::hideEvent(QHideEvent *event) noexcept
{
    QDialog::hideEvent(event);
    emit visibilityChanged(false);
}

// PRIVATE

void FlightDialog::initUi() noexcept
{
    FlightDescriptionWidget *flightDescriptionWidget = new FlightDescriptionWidget(d->flightService, this);
    ui->flightTab->addTab(flightDescriptionWidget, tr("&Description"));

    AircraftTypeWidget *aircraftTypeWidget = new AircraftTypeWidget(this);
    ui->flightTab->addTab(aircraftTypeWidget, tr("&Aircraft"));

    FlightConditionWidget *flightConditionsWidget = new FlightConditionWidget(this);
    ui->flightTab->addTab(flightConditionsWidget, tr("&Conditions"));

    FlightPlanWidget *flightPlanWidget = new FlightPlanWidget(this);
    ui->flightTab->addTab(flightPlanWidget, tr("&Flight Plan"));

    ui->flightTab->setCurrentIndex(0);
}

void FlightDialog::updateUi() noexcept
{}
