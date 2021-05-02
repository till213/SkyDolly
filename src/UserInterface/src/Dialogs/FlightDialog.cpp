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
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "../Widgets/AircraftTypeWidget.h"
#include "../Widgets/FlightConditionWidget.h"
#include "FlightDialog.h"
#include "ui_FlightDialog.h"

class FlightDialogPrivate
{
public:
    FlightDialogPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
};

// PUBLIC

FlightDialog::FlightDialog(SkyConnectIntf &skyConnect, QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<FlightDialogPrivate>(skyConnect)),
    ui(std::make_unique<Ui::FlightDialog>())
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    initUi();
}

FlightDialog::~FlightDialog() noexcept
{
}

// PROTECTED

void FlightDialog::showEvent(QShowEvent *event) noexcept
{
    Q_UNUSED(event)
    updateUi();
    emit visibilityChanged(true);
}

void FlightDialog::hideEvent(QHideEvent *event) noexcept
{
    Q_UNUSED(event)
    emit visibilityChanged(false);
}

// PRIVATE

void FlightDialog::initUi() noexcept
{
    AircraftTypeWidget *aircraftTypeWidget = new AircraftTypeWidget(d->skyConnect, this);
    ui->flightTab->addTab(aircraftTypeWidget, tr("&Aircraft"));

    FlightConditionWidget *flightConditionsWidget = new FlightConditionWidget(d->skyConnect, this);
    ui->flightTab->addTab(flightConditionsWidget, tr("&Conditions"));
}

void FlightDialog::updateUi() noexcept
{
}
