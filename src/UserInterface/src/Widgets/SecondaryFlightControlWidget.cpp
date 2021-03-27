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

#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/World.h"
#include "../../../Model/src/Scenario.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/SecondaryFlightControl.h"
#include "../../../Model/src/SecondaryFlightControlData.h"
#include "../../../Model/src/TimeVariableData.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "SecondaryFlightControlWidget.h"
#include "ui_SecondaryFlightControlWidget.h"

class SecondaryFlightControlWidgetPrivate
{
public:
    SecondaryFlightControlWidgetPrivate(const QWidget &widget, SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect),
          ActiveTextColor(widget.palette().color(QPalette::Active, QPalette::WindowText)),
          DisabledTextColor(widget.palette().color(QPalette::Disabled, QPalette::WindowText))
    {}

    SkyConnectIntf &skyConnect;
    const QColor ActiveTextColor;
    const QColor DisabledTextColor;
};

// PUBLIC

SecondaryFlightControlWidget::SecondaryFlightControlWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QWidget(parent),
    d(std::make_unique<SecondaryFlightControlWidgetPrivate>(*this, skyConnect)),
    ui(std::make_unique<Ui::SecondaryFlightControlWidget>())
{
    ui->setupUi(this);
    initUi();
}

SecondaryFlightControlWidget::~SecondaryFlightControlWidget()
{
}

// PROTECTED

void SecondaryFlightControlWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi(d->skyConnect.getCurrentTimestamp(), TimeVariableData::Access::Seek);

    const SecondaryFlightControl &SecondaryFlightControl = World::getInstance().getCurrentScenario().getUserAircraft().getSecondaryFlightControlConst();
    // Signal sent while recording
    connect(&SecondaryFlightControl, &SecondaryFlightControl::dataChanged,
            this, &SecondaryFlightControlWidget::handleRecordedData);
    // Signal sent while playing
    connect(&d->skyConnect, &SkyConnectIntf::currentTimestampChanged,
            this, &SecondaryFlightControlWidget::handleTimestampChanged);
}

void SecondaryFlightControlWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const SecondaryFlightControl &SecondaryFlightControl = World::getInstance().getCurrentScenario().getUserAircraft().getSecondaryFlightControlConst();
    disconnect(&SecondaryFlightControl, &SecondaryFlightControl::dataChanged,
               this, &SecondaryFlightControlWidget::handleRecordedData);
    disconnect(&d->skyConnect, &SkyConnectIntf::currentTimestampChanged,
            this, &SecondaryFlightControlWidget::handleTimestampChanged);
}

// PRIVATE

void SecondaryFlightControlWidget::initUi()
{
    ui->leadingEdgeFlapsLeftLineEdit->setToolTip(SimVar::LeadingEdgeFlapsLeftPercent);
    ui->leadingEdgeFlapsRightLineEdit->setToolTip(SimVar::LeadingEdgeFlapsRightPercent);
    ui->trailingEdgeFlapsLeftLineEdit->setToolTip(SimVar::TrailingEdgeFlapsLeftPercent);
    ui->trailingEdgeFlapsRightLineEdit->setToolTip(SimVar::TrailingEdgeFlapsRightPercent);
    ui->flapsPositionLineEdit->setToolTip(SimVar::FlapsHandleIndex);
    ui->spoilerLineEdit->setToolTip(SimVar::SpoilersHandlePosition);
}

void SecondaryFlightControlWidget::updateUi(qint64 timestamp, TimeVariableData::Access access)
{
    const SecondaryFlightControlData &secondaryFlightControlData = getCurrentSecondaryFlightControlData(timestamp, access);
    QString colorName;

    if (!secondaryFlightControlData.isNull()) {
        // Flaps & speed brakes
        ui->leadingEdgeFlapsLeftLineEdit->setText(QString::number(secondaryFlightControlData.leadingEdgeFlapsLeftPercent));
        ui->leadingEdgeFlapsRightLineEdit->setText(QString::number(secondaryFlightControlData.leadingEdgeFlapsRightPercent));
        ui->trailingEdgeFlapsLeftLineEdit->setText(QString::number(secondaryFlightControlData.trailingEdgeFlapsLeftPercent));
        ui->trailingEdgeFlapsRightLineEdit->setText(QString::number(secondaryFlightControlData.trailingEdgeFlapsRightPercent));
        ui->spoilerLineEdit->setText(QString::number(secondaryFlightControlData.spoilersHandlePosition));
        ui->flapsPositionLineEdit->setText(QString::number(secondaryFlightControlData.flapsHandleIndex));

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QString("color: %1;").arg(colorName)};
    ui->leadingEdgeFlapsLeftLineEdit->setStyleSheet(css);
    ui->leadingEdgeFlapsRightLineEdit->setStyleSheet(css);
    ui->trailingEdgeFlapsLeftLineEdit->setStyleSheet(css);
    ui->trailingEdgeFlapsRightLineEdit->setStyleSheet(css);
    ui->spoilerLineEdit->setStyleSheet(css);
    ui->flapsPositionLineEdit->setStyleSheet(css);
}

const SecondaryFlightControlData &SecondaryFlightControlWidget::getCurrentSecondaryFlightControlData(qint64 timestamp, TimeVariableData::Access access) const
{
    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();

    if (d->skyConnect.getState() == Connect::State::Recording) {
        return aircraft.getSecondaryFlightControlConst().getLastSecondaryFlightControlData();
    } else {
        if (timestamp != TimeVariableData::InvalidTimestamp) {
            return aircraft.getSecondaryFlightControlConst().interpolateSecondaryFlightControlData(timestamp, access);
        } else {
            return aircraft.getSecondaryFlightControlConst().interpolateSecondaryFlightControlData(d->skyConnect.getCurrentTimestamp(), access);
        }
    };
}

// PRIVATE SLOTS

void SecondaryFlightControlWidget::handleRecordedData()
{
    updateUi(TimeVariableData::InvalidTimestamp, TimeVariableData::Access::Linear);
}

void SecondaryFlightControlWidget::handleTimestampChanged(qint64 timestamp, TimeVariableData::Access access)
{
    updateUi(timestamp, access);
}
