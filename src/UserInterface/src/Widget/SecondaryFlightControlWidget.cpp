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

#include <Kernel/Unit.h>
#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/SecondaryFlightControl.h>
#include <Model/SecondaryFlightControlData.h>
#include <Model/TimeVariableData.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/Connect.h>
#include "SecondaryFlightControlWidget.h"
#include "ui_SecondaryFlightControlWidget.h"

struct SecondaryFlightControlWidgetPrivate
{
    SecondaryFlightControlWidgetPrivate(const QWidget &widget) noexcept
        : ActiveTextColor(widget.palette().color(QPalette::Active, QPalette::WindowText)),
          DisabledTextColor(widget.palette().color(QPalette::Disabled, QPalette::WindowText))
    {}

    Unit unit;
    const QColor ActiveTextColor;
    const QColor DisabledTextColor;
};

// PUBLIC

SecondaryFlightControlWidget::SecondaryFlightControlWidget(QWidget *parent) noexcept :
    AbstractSimulationVariableWidget(parent),
    ui(std::make_unique<Ui::SecondaryFlightControlWidget>()),
    d(std::make_unique<SecondaryFlightControlWidgetPrivate>(*this))
{
    ui->setupUi(this);
    initUi();
}

SecondaryFlightControlWidget::~SecondaryFlightControlWidget() = default;

// PROTECTED SLOTS

void SecondaryFlightControlWidget::updateUi(std::int64_t timestamp, TimeVariableData::Access access) noexcept
{
    const SecondaryFlightControlData &secondaryFlightControlData = getCurrentSecondaryFlightControlData(timestamp, access);
    QString colorName;

    if (!secondaryFlightControlData.isNull()) {
        // Flaps & spoilers (speed brakes)
        ui->flapsHandleIndexLineEdit->setText(QString::number(secondaryFlightControlData.flapsHandleIndex));
        ui->leftLeadingEdgeFlapsLineEdit->setText(d->unit.leftLeadingEdgeFlapsPosition(secondaryFlightControlData.leftLeadingEdgeFlapsPosition));
        ui->rightLeadingEdgeFlapsLineEdit->setText(d->unit.leftLeadingEdgeFlapsPosition(secondaryFlightControlData.rightLeadingEdgeFlapsPosition));
        ui->leftTrailingEdgeFlapsLineEdit->setText(d->unit.leftLeadingEdgeFlapsPosition(secondaryFlightControlData.leftTrailingEdgeFlapsPosition));
        ui->rightTrailingEdgeFlapsLineEdit->setText(d->unit.leftLeadingEdgeFlapsPosition(secondaryFlightControlData.rightTrailingEdgeFlapsPosition));

        ui->spoilersHandlePositionLineEdit->setText(d->unit.formatPercent(secondaryFlightControlData.spoilersHandlePercent));
        ui->leftSpoilersPositionLineEdit->setText(d->unit.leftLeadingEdgeFlapsPosition(secondaryFlightControlData.leftSpoilersPosition));
        ui->rightSpoilersPositionLineEdit->setText(d->unit.leftLeadingEdgeFlapsPosition(secondaryFlightControlData.rightSpoilersPosition));

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QString("color: %1;").arg(colorName)};
    ui->flapsHandleIndexLineEdit->setStyleSheet(css);
    ui->leftLeadingEdgeFlapsLineEdit->setStyleSheet(css);
    ui->rightLeadingEdgeFlapsLineEdit->setStyleSheet(css);
    ui->leftTrailingEdgeFlapsLineEdit->setStyleSheet(css);
    ui->rightTrailingEdgeFlapsLineEdit->setStyleSheet(css);

    ui->spoilersHandlePositionLineEdit->setStyleSheet(css);
    ui->leftSpoilersPositionLineEdit->setStyleSheet(css);
    ui->rightSpoilersPositionLineEdit->setStyleSheet(css);
}

// PRIVATE

void SecondaryFlightControlWidget::initUi() noexcept
{
    ui->flapsHandleIndexLineEdit->setToolTip(SimVar::FlapsHandleIndex);
    ui->leftLeadingEdgeFlapsLineEdit->setToolTip(SimVar::LeadingEdgeFlapsLeftPercent);
    ui->rightLeadingEdgeFlapsLineEdit->setToolTip(SimVar::LeadingEdgeFlapsRightPercent);
    ui->leftTrailingEdgeFlapsLineEdit->setToolTip(SimVar::TrailingEdgeFlapsLeftPercent);
    ui->rightTrailingEdgeFlapsLineEdit->setToolTip(SimVar::TrailingEdgeFlapsRightPercent);

    ui->spoilersHandlePositionLineEdit->setToolTip(SimVar::SpoilersHandlePosition);
    ui->leftSpoilersPositionLineEdit->setToolTip(SimVar::SpoilersLeftPosition);
    ui->rightSpoilersPositionLineEdit->setToolTip(SimVar::SpoilersRightPosition);
}

SecondaryFlightControlData SecondaryFlightControlWidget::getCurrentSecondaryFlightControlData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    SecondaryFlightControlData secondaryFlightControlData;
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().getState() == Connect::State::Recording) {
            if (aircraft.getSecondaryFlightControl().count() > 0) {
                secondaryFlightControlData = aircraft.getSecondaryFlightControl().getLast();
            }
        } else {
            if (timestamp != TimeVariableData::InvalidTime) {
                secondaryFlightControlData = aircraft.getSecondaryFlightControl().interpolate(timestamp, access);
            } else {
                secondaryFlightControlData = aircraft.getSecondaryFlightControl().interpolate(skyConnect->get().getCurrentTimestamp(), access);
            }
        };
    }
    return secondaryFlightControlData;
}
