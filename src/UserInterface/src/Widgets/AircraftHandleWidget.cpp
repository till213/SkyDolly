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
#include "../../../Model/src/AircraftHandle.h"
#include "../../../Model/src/AircraftHandleData.h"
#include "../../../Model/src/TimeVariableData.h"
#include "../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "AircraftHandleWidget.h"
#include "ui_AircraftHandleWidget.h"

class AircraftHandleWidgetPrivate
{
public:
    AircraftHandleWidgetPrivate(const QWidget &widget) noexcept
        : ActiveTextColor(widget.palette().color(QPalette::Active, QPalette::WindowText)),
          DisabledTextColor(widget.palette().color(QPalette::Disabled, QPalette::WindowText))
    {}

    Unit unit;
    const QColor ActiveTextColor;
    const QColor DisabledTextColor;
};

// PUBLIC

AircraftHandleWidget::AircraftHandleWidget(QWidget *parent) noexcept :
    AbstractSimulationVariableWidget(parent),
    d(std::make_unique<AircraftHandleWidgetPrivate>(*this)),
    ui(std::make_unique<Ui::AircraftHandleWidget>())
{
    ui->setupUi(this);
    initUi();
}

AircraftHandleWidget::~AircraftHandleWidget() noexcept
{}

// PROTECTED


// PRIVATE

void AircraftHandleWidget::initUi() noexcept
{
    ui->gearLineEdit->setToolTip(SimVar::GearHandlePosition);
    ui->brakeLeftLineEdit->setToolTip(SimVar::BrakeLeftPosition);
    ui->brakeRightLineEdit->setToolTip(SimVar::BrakeRightPosition);
    ui->waterRudderLineEdit->setToolTip(SimVar::WaterRudderHandlePosition);
    ui->tailhookLineEdit->setToolTip(SimVar::TailhookPosition);
    ui->canopyOpenLineEdit->setToolTip(SimVar::CanopyOpen);
    ui->leftWingFoldingLineEdit->setToolTip(SimVar::FoldingWingLeftPercent);
    ui->rightWingFoldingLineEdit->setToolTip(SimVar::FoldingWingRightPercent);
}

const AircraftHandleData &AircraftHandleWidget::getCurrentAircraftHandleData(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    const auto skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().getState() == Connect::State::Recording) {
            return aircraft.getAircraftHandleConst().getLast();
        } else {
            if (timestamp != TimeVariableData::InvalidTime) {
                return aircraft.getAircraftHandleConst().interpolate(timestamp, access);
            } else {
                return aircraft.getAircraftHandleConst().interpolate(skyConnect->get().getCurrentTimestamp(), access);
            }
        };
    } else {
        return AircraftHandleData::NullData;
    }
}

// PRIVATE SLOTS

void AircraftHandleWidget::updateUi(qint64 timestamp, TimeVariableData::Access access) noexcept
{
    const AircraftHandleData &aircraftHandleData = getCurrentAircraftHandleData(timestamp, access);
    QString colorName;

    if (!aircraftHandleData.isNull()) {
        aircraftHandleData.gearHandlePosition ? ui->gearLineEdit->setText(tr("Down")) : ui->gearLineEdit->setText(tr("Up"));
        ui->brakeLeftLineEdit->setText(d->unit.formatPosition(aircraftHandleData.brakeLeftPosition));
        ui->brakeRightLineEdit->setText(d->unit.formatPosition(aircraftHandleData.brakeRightPosition));
        ui->waterRudderLineEdit->setText(d->unit.formatPosition(aircraftHandleData.waterRudderHandlePosition));
        ui->tailhookLineEdit->setText(d->unit.formatPercent(aircraftHandleData.tailhookPosition));
        ui->canopyOpenLineEdit->setText(d->unit.formatPercent(aircraftHandleData.canopyOpen));
        ui->leftWingFoldingLineEdit->setText(d->unit.formatPercent(aircraftHandleData.leftWingFolding));
        ui->rightWingFoldingLineEdit->setText(d->unit.formatPercent(aircraftHandleData.rightWingFolding));

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QString("color: %1;").arg(colorName)};
    ui->gearLineEdit->setStyleSheet(css);
    ui->brakeLeftLineEdit->setStyleSheet(css);
    ui->brakeRightLineEdit->setStyleSheet(css);
    ui->waterRudderLineEdit->setStyleSheet(css);
    ui->tailhookLineEdit->setStyleSheet(css);
    ui->canopyOpenLineEdit->setStyleSheet(css);
    ui->leftWingFoldingLineEdit->setStyleSheet(css);
    ui->rightWingFoldingLineEdit->setStyleSheet(css);
}
