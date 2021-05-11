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

#include <QString>
#include <QDialog>
#include <QColor>
#include <QPalette>

#include "../../../Kernel/src/Unit.h"
#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/PrimaryFlightControl.h"
#include "../../../Model/src/PrimaryFlightControlData.h"
#include "../../../Model/src/TimeVariableData.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "PrimaryFlightControlWidget.h"
#include "ui_PrimaryFlightControlWidget.h"

class PrimaryFlightControlWidgetPrivate
{
public:
    PrimaryFlightControlWidgetPrivate(const QWidget &widget, SkyConnectIntf &theSkyConnect) noexcept
        : skyConnect(theSkyConnect),
          ActiveTextColor(widget.palette().color(QPalette::Active, QPalette::WindowText)),
          DisabledTextColor(widget.palette().color(QPalette::Disabled, QPalette::WindowText))
    {}

    SkyConnectIntf &skyConnect;
    Unit unit;
    const QColor ActiveTextColor;
    const QColor DisabledTextColor;
};

// PUBLIC

PrimaryFlightControlWidget::PrimaryFlightControlWidget(SkyConnectIntf &skyConnect, QWidget *parent) noexcept :
    QWidget(parent),
    d(std::make_unique<PrimaryFlightControlWidgetPrivate>(*this, skyConnect)),
    ui(std::make_unique<Ui::PrimaryFlightControlWidget>())
{
    ui->setupUi(this);
    initUi();
}

PrimaryFlightControlWidget::~PrimaryFlightControlWidget() noexcept
{}

// PROTECTED

void PrimaryFlightControlWidget::showEvent(QShowEvent *event) noexcept
{
    Q_UNUSED(event)

    updateUi(d->skyConnect.getCurrentTimestamp(), TimeVariableData::Access::Seek);
    connect(&d->skyConnect, &SkyConnectIntf::timestampChanged,
            this, &PrimaryFlightControlWidget::updateUi);
}

void PrimaryFlightControlWidget::hideEvent(QHideEvent *event) noexcept
{
    Q_UNUSED(event)

    disconnect(&d->skyConnect, &SkyConnectIntf::timestampChanged,
            this, &PrimaryFlightControlWidget::updateUi);
}

// PRIVATE

void PrimaryFlightControlWidget::initUi()
{
    ui->rudderLineEdit->setToolTip(SimVar::RudderPosition);
    ui->elevatorLineEdit->setToolTip(SimVar::ElevatorPosition);
    ui->aileronLineEdit->setToolTip(SimVar::AileronPosition);
}

const PrimaryFlightControlData &PrimaryFlightControlWidget::getCurrentPrimaryFlightControlData(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    if (d->skyConnect.getState() == Connect::State::Recording) {
        return aircraft.getPrimaryFlightControlConst().getLast();
    } else {
        if (timestamp != TimeVariableData::InvalidTime) {
            return aircraft.getPrimaryFlightControlConst().interpolate(timestamp, access);
        } else {
            return aircraft.getPrimaryFlightControlConst().interpolate(d->skyConnect.getCurrentTimestamp(), access);
        }
    };
}

// PRIVATE SLOTS

void PrimaryFlightControlWidget::updateUi(qint64 timestamp, TimeVariableData::Access access) noexcept
{
    const PrimaryFlightControlData &primaryFlightControlData = getCurrentPrimaryFlightControlData(timestamp, access);
    QString colorName;

    if (!primaryFlightControlData.isNull()) {
        ui->rudderLineEdit->setText(d->unit.formatPosition(primaryFlightControlData.rudderPosition));
        ui->elevatorLineEdit->setText(d->unit.formatPosition(primaryFlightControlData.elevatorPosition));
        ui->aileronLineEdit->setText(d->unit.formatPosition(primaryFlightControlData.aileronPosition));

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QString("color: %1;").arg(colorName)};
    ui->rudderLineEdit->setStyleSheet(css);
    ui->elevatorLineEdit->setStyleSheet(css);
    ui->aileronLineEdit->setStyleSheet(css);
}
