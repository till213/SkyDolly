/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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

#include <QWidget>
#include <QDialog>
#include <QShortcut>
#include <QShowEvent>
#include <QHideEvent>

#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/PrimaryFlightControl.h"
#include "../../../Model/src/AircraftInfo.h"
#include "../../../Persistence/src/Service/FlightService.h"
#include "../Widget/FlightDescriptionWidget.h"
#include "../Widget/AircraftTypeWidget.h"
#include "../Widget/FlightConditionWidget.h"
#include "../Widget/FlightPlanWidget.h"
#include "FlightDialog.h"
#include "ui_FlightDialog.h"

class FlightDialogPrivate
{
public:
    FlightDialogPrivate(FlightService &theFlightService) noexcept
        : flightService(theFlightService),
          closeDialogShortcut(nullptr)
    {}

    FlightService &flightService;
    QShortcut *closeDialogShortcut;
};

// PUBLIC

FlightDialog::FlightDialog(FlightService &flightService, QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<FlightDialogPrivate>(flightService)),
    ui(std::make_unique<Ui::FlightDialog>())
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
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
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    FlightDescriptionWidget *flightDescriptionWidget = new FlightDescriptionWidget(d->flightService, this);
    ui->flightTab->addTab(flightDescriptionWidget, tr("&Description"));

    AircraftTypeWidget *aircraftTypeWidget = new AircraftTypeWidget(this);
    ui->flightTab->addTab(aircraftTypeWidget, tr("&Aircraft"));

    FlightConditionWidget *flightConditionsWidget = new FlightConditionWidget(this);
    ui->flightTab->addTab(flightConditionsWidget, tr("&Conditions"));

    FlightPlanWidget *flightPlanWidget = new FlightPlanWidget(this);
    ui->flightTab->addTab(flightPlanWidget, tr("&Flight Plan"));

    ui->flightTab->setCurrentIndex(0);

    // TODO DRY: "centrally" define the "F" shortcut (currently also assigned to the corresponding QAction)
    d->closeDialogShortcut = new QShortcut(QKeySequence(tr("F", "Window|Flight...")), this);
}

void FlightDialog::updateUi() noexcept
{}

void FlightDialog::frenchConnection() noexcept
{
    connect(d->closeDialogShortcut, &QShortcut::activated,
            this, &FlightDialog::close);
}
