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
 * to whom the Software is furnished to do so, subject to the following Plans:
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
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/FlightPlan.h"
#include "../../../Model/src/FlightPlanData.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "WaypointWidget.h"

#include "FlightPlanWidget.h"
#include "ui_FlightPlanWidget.h"

class FlightPlanWidgetPrivate
{
public:
    FlightPlanWidgetPrivate(SkyConnectIntf &theSkyConnect) noexcept
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
};

// PUBLIC

FlightPlanWidget::FlightPlanWidget(SkyConnectIntf &skyConnect, QWidget *parent) noexcept :
    QWidget(parent),
    d(std::make_unique<FlightPlanWidgetPrivate>(skyConnect)),
    ui(new Ui::FlightPlanWidget)
{
    ui->setupUi(this);
}

FlightPlanWidget::~FlightPlanWidget() noexcept
{
}

// PROTECTED

void FlightPlanWidget::showEvent(QShowEvent *event) noexcept
{
    Q_UNUSED(event)

    updateUi();

    const Flight &currentFlight = Logbook::getInstance().getCurrentFlight();
    connect(&currentFlight, &Flight::aircraftDataChanged,
            this, &FlightPlanWidget::updateUi);
}

void FlightPlanWidget::hideEvent(QHideEvent *event) noexcept
{
    Q_UNUSED(event)

    const Flight &currentFlight = Logbook::getInstance().getCurrentFlight();
    disconnect(&currentFlight, &Flight::aircraftDataChanged,
            this, &FlightPlanWidget::updateUi);
}

// PRIVATE

void FlightPlanWidget::initUi() noexcept
{}

// PRIVATE SLOTS

void FlightPlanWidget::updateUi() noexcept
{
    const Flight &currentFlight = Logbook::getInstance().getCurrentFlight();
    const FlightPlan &flightPlan = currentFlight.getUserAircraftConst().getFlightPlanConst();

    ui->waypointTabWidget->clear();
    for (const FlightPlanData &data : flightPlan.getAllConst()) {
        WaypointWidget *waypointWidget = new WaypointWidget(data, ui->waypointTabWidget);
        ui->waypointTabWidget->addTab(waypointWidget, data.waypointIdentifier);
    }
}
