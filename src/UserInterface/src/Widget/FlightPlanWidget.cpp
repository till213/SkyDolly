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
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Aircraft.h>
#include <Model/Flight.h>
#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include "WaypointWidget.h"

#include "FlightPlanWidget.h"
#include "ui_FlightPlanWidget.h"

struct FlightPlanWidgetPrivate
{};

// PUBLIC

FlightPlanWidget::FlightPlanWidget(QWidget *parent) noexcept :
    QWidget {parent},
    ui {std::make_unique<Ui::FlightPlanWidget>()},
    d {std::make_unique<FlightPlanWidgetPrivate>()}
{
    ui->setupUi(this);
}

FlightPlanWidget::~FlightPlanWidget() = default;

// PROTECTED

void FlightPlanWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);
    updateUi();

    const auto &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::waypointAdded,
            this, &FlightPlanWidget::addWaypoint);
    connect(&flight, &Flight::waypointUpdated,
            this, &FlightPlanWidget::updateWaypoint);
    connect(&flight, &Flight::waypointsCleared,
            this, &FlightPlanWidget::clear);
    connect(&flight, &Flight::userAircraftChanged,
            this, &FlightPlanWidget::updateUi);
    connect(&flight, &Flight::flightStored,
            this, &FlightPlanWidget::updateUi);
    connect(&flight, &Flight::flightRestored,
            this, &FlightPlanWidget::updateUi);

}

void FlightPlanWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::waypointAdded,
               this, &FlightPlanWidget::addWaypoint);
    disconnect(&flight, &Flight::waypointUpdated,
               this, &FlightPlanWidget::updateWaypoint);
    disconnect(&flight, &Flight::waypointsCleared,
               this, &FlightPlanWidget::clear);
    disconnect(&flight, &Flight::userAircraftChanged,
               this, &FlightPlanWidget::updateUi);
    disconnect(&flight, &Flight::flightStored,
               this, &FlightPlanWidget::updateUi);
    disconnect(&flight, &Flight::flightRestored,
               this, &FlightPlanWidget::updateUi);
}

// PRIVATE

void FlightPlanWidget::initUi() noexcept
{}

// PRIVATE SLOTS

void FlightPlanWidget::updateUi() noexcept
{
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    const auto &flightPlan = flight.getUserAircraft().getFlightPlan();

    clear();
    for (const auto &waypoint : flightPlan) {
        addWaypoint(waypoint);
    }
}

void FlightPlanWidget::addWaypoint(const Waypoint &waypoint)
{
    auto waypointWidget = new WaypointWidget(waypoint, ui->waypointTabWidget);
    ui->waypointTabWidget->addTab(waypointWidget, waypoint.identifier);
    ui->waypointTabWidget->setCurrentWidget(waypointWidget);
}

void FlightPlanWidget::updateWaypoint(int index, const Waypoint &waypoint)
{
    auto waypointWidget = dynamic_cast<WaypointWidget *>(ui->waypointTabWidget->widget(index));
    waypointWidget->update(waypoint);
}

void FlightPlanWidget::clear()
{
    while (ui->waypointTabWidget->count() > 0) {
        auto tabWidget = ui->waypointTabWidget->currentWidget();
        delete tabWidget;
    }
}
