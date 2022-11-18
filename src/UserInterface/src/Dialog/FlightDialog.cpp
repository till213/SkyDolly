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
#include "FlightDialog.h"
#include "ui_FlightDialog.h"

#include <memory>

#include <QWidget>
#include <QDialog>
#include <QShortcut>
#include <QShowEvent>
#include <QHideEvent>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Aircraft.h>
#include <Model/PrimaryFlightControl.h>
#include <Model/AircraftInfo.h>
#include <Persistence/Service/FlightService.h>
#include "../Widget/FlightDescriptionWidget.h"
#include "../Widget/AircraftInfoWidget.h"
#include "../Widget/FlightConditionWidget.h"
#include "../Widget/FlightPlanWidget.h"
#include "FlightDialog.h"
#include "ui_FlightDialog.h"

struct FlightDialogPrivate
{
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
#ifdef DEBUG
    qDebug() << "FlightDialog::FlightDialog: CREATED";
#endif
}

FlightDialog::~FlightDialog()
{
#ifdef DEBUG
    qDebug() << "FlightDialog::~FlightDialog: DELETED";
#endif
}

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

/// \todo DRY: "centrally" define the "F" shortcut (currently also assigned to the corresponding QAction)
void FlightDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    FlightDescriptionWidget *flightDescriptionWidget = new FlightDescriptionWidget(d->flightService, this);
    ui->flightTab->addTab(flightDescriptionWidget, tr("&Description"));

    AircraftInfoWidget *aircraftInfoWidget = new AircraftInfoWidget(this);
    ui->flightTab->addTab(aircraftInfoWidget, tr("&Aircraft"));

    FlightConditionWidget *flightConditionsWidget = new FlightConditionWidget(this);
    ui->flightTab->addTab(flightConditionsWidget, tr("&Conditions"));

    FlightPlanWidget *flightPlanWidget = new FlightPlanWidget(this);
    ui->flightTab->addTab(flightPlanWidget, tr("&Flight Plan"));

    ui->flightTab->setCurrentIndex(0);

    d->closeDialogShortcut = new QShortcut(QKeySequence(tr("F", "Window|Flight...")), this);
}

void FlightDialog::updateUi() noexcept
{}

void FlightDialog::frenchConnection() noexcept
{
    connect(d->closeDialogShortcut, &QShortcut::activated,
            this, &FlightDialog::close);
}
