/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

#include <QStringBuilder>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Const.h>
#include <Kernel/Unit.h>
#include <Model/Waypoint.h>
#include <Model/SimVar.h>
#include "WaypointWidget.h"
#include "ui_WaypointWidget.h"

struct WaypointWidgetPrivate
{
    WaypointWidgetPrivate(Waypoint waypoint) noexcept
        : waypoint(std::move(waypoint))
    {}

    Waypoint waypoint;
    Unit unit;
};

// PUBLIC

WaypointWidget::WaypointWidget(Waypoint waypoint, QWidget *parent) noexcept :
    QWidget {parent},
    ui {std::make_unique<Ui::WaypointWidget>()},
    d {std::make_unique<WaypointWidgetPrivate>(std::move(waypoint))}
{
    ui->setupUi(this);
    initUi();
}

WaypointWidget::~WaypointWidget() = default;

void WaypointWidget::update(Waypoint waypoint) noexcept
{
    d->waypoint = std::move(waypoint);
    updateUi();
}

// PROTECTED

void WaypointWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);
    updateUi();
}

void WaypointWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);
}

// PRIVATE

void WaypointWidget::initUi() noexcept
{
    ui->latitudeLineEdit->setToolTip(SimVar::GpsWPPrevLat);
    ui->longitudeLineEdit->setToolTip(SimVar::GpsWPPrevLon);
    ui->altitudeLineEdit->setToolTip(SimVar::GpsWPPrevAlt);
}

// PRIVATE SLOTS

void WaypointWidget::updateUi() noexcept
{
    ui->latitudeLineEdit->setText(d->unit.formatLatitudeDMS(d->waypoint.latitude));
    ui->longitudeLineEdit->setText(d->unit.formatLongitudeDMS(d->waypoint.longitude));
    ui->altitudeLineEdit->setText(d->unit.formatFeet(d->waypoint.altitude));
    ui->localSimulationTimeLineEdit->setText(d->unit.formatDateTime(d->waypoint.localTime));
    ui->localSimulationTimeLineEdit->setToolTip(d->unit.formatDateTime(d->waypoint.zuluTime) % Const::ZuluTimeSuffix);
}
