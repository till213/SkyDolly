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
#include <memory>
#include <cstdint>

#include <QString>
#include <QDialog>
#include <QColor>
#include <QPalette>

#include <Kernel/Convert.h>
#include <Kernel/Unit.h>
#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/PrimaryFlightControl.h>
#include <Model/PrimaryFlightControlData.h>
#include <Model/TimeVariableData.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/Connect.h>
#include "PrimaryFlightControlWidget.h"
#include "ui_PrimaryFlightControlWidget.h"

struct PrimaryFlightControlWidgetPrivate
{
    PrimaryFlightControlWidgetPrivate(const QWidget &widget) noexcept
        : ActiveTextColor(widget.palette().color(QPalette::Active, QPalette::WindowText)),
          DisabledTextColor(widget.palette().color(QPalette::Disabled, QPalette::WindowText))
    {}

    Unit unit;
    const QColor ActiveTextColor;
    const QColor DisabledTextColor;
};

// PUBLIC

PrimaryFlightControlWidget::PrimaryFlightControlWidget(QWidget *parent) noexcept :
    AbstractSimulationVariableWidget(parent),
    ui(std::make_unique<Ui::PrimaryFlightControlWidget>()),
    d(std::make_unique<PrimaryFlightControlWidgetPrivate>(*this))
{
    ui->setupUi(this);
    initUi();
}

PrimaryFlightControlWidget::~PrimaryFlightControlWidget() = default;

// PROTECED SLOTS

void PrimaryFlightControlWidget::updateUi(std::int64_t timestamp, TimeVariableData::Access access) noexcept
{
    const PrimaryFlightControlData &primaryFlightControlData = getCurrentPrimaryFlightControlData(timestamp, access);
    QString colorName;

    if (!primaryFlightControlData.isNull()) {
        ui->rudderDeflectionLineEdit->setText(d->unit.formatDegrees(Convert::radiansToDegrees(primaryFlightControlData.rudderDeflection)));
        ui->elevatorDeflectionLineEdit->setText(d->unit.formatDegrees(Convert::radiansToDegrees(primaryFlightControlData.elevatorDeflection)));
        ui->leftAileronDeflectionLineEdit->setText(d->unit.formatDegrees(Convert::radiansToDegrees(primaryFlightControlData.leftAileronDeflection)));
        ui->rightAileronDeflectionLineEdit->setText(d->unit.formatDegrees(Convert::radiansToDegrees(primaryFlightControlData.rightAileronDeflection)));
        ui->rudderLineEdit->setText(d->unit.leftLeadingEdgeFlapsPosition(primaryFlightControlData.rudderPosition));
        ui->elevatorLineEdit->setText(d->unit.leftLeadingEdgeFlapsPosition(primaryFlightControlData.elevatorPosition));
        ui->aileronLineEdit->setText(d->unit.leftLeadingEdgeFlapsPosition(primaryFlightControlData.aileronPosition));

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QString("color: %1;").arg(colorName)};
    ui->rudderDeflectionLineEdit->setStyleSheet(css);
    ui->elevatorDeflectionLineEdit->setStyleSheet(css);
    ui->leftAileronDeflectionLineEdit->setStyleSheet(css);
    ui->rightAileronDeflectionLineEdit->setStyleSheet(css);
    ui->rudderLineEdit->setStyleSheet(css);
    ui->elevatorLineEdit->setStyleSheet(css);
    ui->aileronLineEdit->setStyleSheet(css);
}

// PRIVATE

void PrimaryFlightControlWidget::initUi()
{
    ui->rudderDeflectionLineEdit->setToolTip(SimVar::RudderDeflection);
    ui->elevatorDeflectionLineEdit->setToolTip(SimVar::ElevatorDeflection);
    ui->leftAileronDeflectionLineEdit->setToolTip(SimVar::AileronLeftDeflection);
    ui->rightAileronDeflectionLineEdit->setToolTip(SimVar::AileronRightDeflection);
    ui->rudderLineEdit->setToolTip(SimVar::RudderPosition);
    ui->elevatorLineEdit->setToolTip(SimVar::ElevatorPosition);
    ui->aileronLineEdit->setToolTip(SimVar::AileronPosition);
}

PrimaryFlightControlData PrimaryFlightControlWidget::getCurrentPrimaryFlightControlData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    PrimaryFlightControlData primaryFlightControlData;
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().getState() == Connect::State::Recording) {
            if (aircraft.getPrimaryFlightControl().count() > 0) {
                primaryFlightControlData = aircraft.getPrimaryFlightControl().getLast();
            }
        } else {
            if (timestamp != TimeVariableData::InvalidTime) {
                primaryFlightControlData = aircraft.getPrimaryFlightControl().interpolate(timestamp, access);
            } else {
                primaryFlightControlData = aircraft.getPrimaryFlightControl().interpolate(skyConnect->get().getCurrentTimestamp(), access);
            }
        };
    }
    return primaryFlightControlData;
}
