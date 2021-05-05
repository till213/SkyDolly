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

#include "../../../Model/src/FlightPlanData.h"
#include "../../../Model/src/SimVar.h"
#include "../Unit.h"
#include "WaypointWidget.h"
#include "ui_WaypointWidget.h"

class WaypointWidgetPrivate
{
public:
    WaypointWidgetPrivate(const FlightPlanData &theFlightPlanData) noexcept
        : flightPlanData(theFlightPlanData)
    {}

    const FlightPlanData &flightPlanData;
    Unit unit;
};

// PUBLIC

WaypointWidget::WaypointWidget(const FlightPlanData &flightPlanData, QWidget *parent) noexcept :
    QWidget(parent),
    d(std::make_unique<WaypointWidgetPrivate>(flightPlanData)),
    ui(std::make_unique<Ui::WaypointWidget>())
{
    ui->setupUi(this);
    initUi();
}

WaypointWidget::~WaypointWidget() noexcept
{}

// PROTECTED

void WaypointWidget::showEvent(QShowEvent *event) noexcept
{
    Q_UNUSED(event)
    updateUi();
}

void WaypointWidget::hideEvent(QHideEvent *event) noexcept
{
    Q_UNUSED(event)
}

// PRIVATE

void WaypointWidget::initUi() noexcept
{
    ui->latitudeLineEdit->setToolTip(SimVar::GpsWPPrevLat);
    ui->longitudeLineEdit->setToolTip(SimVar::GpsWPPrevLon);
    ui->altitudeLineEdit->setToolTip(SimVar::GpsWPPrevAlt);
    ui->localSimulationTimeLineEdit->setToolTip(SimVar::LocalTime);
    ui->zuluSimulationTimeLineEdit->setToolTip(SimVar::ZuluTime);
}

// PRIVATE SLOTS

void WaypointWidget::updateUi() noexcept
{
    ui->latitudeLineEdit->setText(d->unit.formatLatitude(d->flightPlanData.latitude));
    ui->longitudeLineEdit->setText(d->unit.formatLatitude(d->flightPlanData.longitude));
    ui->altitudeLineEdit->setText(d->unit.formatFeet(d->flightPlanData.altitude));
    ui->localSimulationTimeLineEdit->setText(d->unit.formatDateTime(d->flightPlanData.localTime));
    ui->zuluSimulationTimeLineEdit->setText(d->unit.formatDateTime(d->flightPlanData.zuluTime));
}
