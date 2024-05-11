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
#include <optional>
#include <cstdint>

#include <QString>
#include <QStringLiteral>
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
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Connect/Connect.h>
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
        ui->leftLeadingEdgeFlapsLineEdit->setText(d->unit.formatPosition(secondaryFlightControlData.leftLeadingEdgeFlapsPosition));
        ui->rightLeadingEdgeFlapsLineEdit->setText(d->unit.formatPosition(secondaryFlightControlData.rightLeadingEdgeFlapsPosition));
        ui->leftTrailingEdgeFlapsLineEdit->setText(d->unit.formatPosition(secondaryFlightControlData.leftTrailingEdgeFlapsPosition));
        ui->rightTrailingEdgeFlapsLineEdit->setText(d->unit.formatPosition(secondaryFlightControlData.rightTrailingEdgeFlapsPosition));

        ui->spoilersHandlePositionLineEdit->setText(d->unit.formatPercent(secondaryFlightControlData.spoilersHandlePercent));
        secondaryFlightControlData.spoilersArmed ? ui->spoilersArmedLineEdit->setText(tr("Armed")) : ui->spoilersArmedLineEdit->setText(tr("Disarmed"));
        ui->leftSpoilersPositionLineEdit->setText(d->unit.formatPosition(secondaryFlightControlData.leftSpoilersPosition));
        ui->rightSpoilersPositionLineEdit->setText(d->unit.formatPosition(secondaryFlightControlData.rightSpoilersPosition));

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QStringLiteral("color: %1;").arg(colorName)};
    ui->flapsHandleIndexLineEdit->setStyleSheet(css);
    ui->leftLeadingEdgeFlapsLineEdit->setStyleSheet(css);
    ui->rightLeadingEdgeFlapsLineEdit->setStyleSheet(css);
    ui->leftTrailingEdgeFlapsLineEdit->setStyleSheet(css);
    ui->rightTrailingEdgeFlapsLineEdit->setStyleSheet(css);

    ui->spoilersHandlePositionLineEdit->setStyleSheet(css);
    ui->spoilersArmedLineEdit->setStyleSheet(css);
    ui->leftSpoilersPositionLineEdit->setStyleSheet(css);
    ui->rightSpoilersPositionLineEdit->setStyleSheet(css);
}

// PRIVATE

void SecondaryFlightControlWidget::initUi() noexcept
{
    ui->flapsHandleIndexLineEdit->setToolTip(QString::fromLatin1(SimVar::FlapsHandleIndex));
    ui->leftLeadingEdgeFlapsLineEdit->setToolTip(QString::fromLatin1(SimVar::LeadingEdgeFlapsLeftPercent));
    ui->rightLeadingEdgeFlapsLineEdit->setToolTip(QString::fromLatin1(SimVar::LeadingEdgeFlapsRightPercent));
    ui->leftTrailingEdgeFlapsLineEdit->setToolTip(
        QString::fromLatin1(SimVar::TrailingEdgeFlapsLeftPercent));
    ui->rightTrailingEdgeFlapsLineEdit->setToolTip(QString::fromLatin1(SimVar::TrailingEdgeFlapsRightPercent));

    ui->spoilersHandlePositionLineEdit->setToolTip(QString::fromLatin1(SimVar::SpoilersHandlePosition));
    ui->spoilersArmedLineEdit->setToolTip(QString::fromLatin1(SimVar::SpoilersArmed));
    ui->leftSpoilersPositionLineEdit->setToolTip(QString::fromLatin1(SimVar::SpoilersLeftPosition));
    ui->rightSpoilersPositionLineEdit->setToolTip(QString::fromLatin1(SimVar::SpoilersRightPosition));
}

SecondaryFlightControlData SecondaryFlightControlWidget::getCurrentSecondaryFlightControlData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    SecondaryFlightControlData data;
    const auto &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const auto &secondaryFlightControl = aircraft.getSecondaryFlightControl();
    const auto &skyConnectManager = SkyConnectManager::getInstance();

    if (skyConnectManager.getState() == Connect::State::Recording) {
        if (secondaryFlightControl.count() > 0) {
            data = secondaryFlightControl.getLast();
        }
    } else {
        const auto t = timestamp != TimeVariableData::InvalidTime ? timestamp : skyConnectManager.getCurrentTimestamp();
        data = secondaryFlightControl.interpolate(t, access);
    };

    return data;
}
