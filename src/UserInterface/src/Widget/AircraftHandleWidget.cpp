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
#include <QStringLiteral>
#include <QDialog>
#include <QColor>
#include <QPalette>

#include <Kernel/Unit.h>
#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/AircraftHandle.h>
#include <Model/AircraftHandleData.h>
#include <Model/TimeVariableData.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Connect/Connect.h>
#include "AircraftHandleWidget.h"
#include "ui_AircraftHandleWidget.h"

struct AircraftHandleWidgetPrivate
{
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
    AbstractSimulationVariableWidget {parent},
    ui {std::make_unique<Ui::AircraftHandleWidget>()},
    d {std::make_unique<AircraftHandleWidgetPrivate>(*this)}
{
    ui->setupUi(this);
    initUi();
}

AircraftHandleWidget::~AircraftHandleWidget() = default;

// PRIVATE

void AircraftHandleWidget::initUi() noexcept
{
    ui->canopyOpenLineEdit->setToolTip(SimVar::CanopyOpen);
    ui->gearLineEdit->setToolTip(SimVar::GearHandlePosition);
    ui->brakeLeftLineEdit->setToolTip(SimVar::BrakeLeftPosition);
    ui->brakeRightLineEdit->setToolTip(SimVar::BrakeRightPosition);
    ui->steerInputControlLineEdit->setToolTip(SimVar::SteerInputControl);
    ui->waterRudderLineEdit->setToolTip(SimVar::WaterRudderHandlePosition);    
    ui->tailhookHandleLineEdit->setToolTip(SimVar::TailhookHandle);
    ui->tailhookPositionLineEdit->setToolTip(SimVar::TailhookPosition);
    ui->wingFoldingHandleLineEdit->setToolTip(SimVar::FoldingWingHandlePosition);
    ui->leftWingFoldingLineEdit->setToolTip(SimVar::FoldingWingLeftPercent);
    ui->rightWingFoldingLineEdit->setToolTip(SimVar::FoldingWingRightPercent);
}

AircraftHandleData AircraftHandleWidget::getCurrentAircraftHandleData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    AircraftHandleData data;
    const auto &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const auto &aircraftHandle = aircraft.getAircraftHandle();
    const auto &skyConnectManager = SkyConnectManager::getInstance();

    if (skyConnectManager.getState() == Connect::State::Recording) {
        if (aircraftHandle.count() > 0) {
            data = aircraftHandle.getLast();
        }
    } else {
        const auto t = timestamp != TimeVariableData::InvalidTime ? timestamp : skyConnectManager.getCurrentTimestamp();
        data = aircraftHandle.interpolate(t, access);
    };

    return data;
}

// PRIVATE SLOTS

void AircraftHandleWidget::updateUi(std::int64_t timestamp, TimeVariableData::Access access) noexcept
{
    const AircraftHandleData &aircraftHandleData = getCurrentAircraftHandleData(timestamp, access);
    QString colorName;

    if (!aircraftHandleData.isNull()) {
        ui->canopyOpenLineEdit->setText(d->unit.formatPercent(aircraftHandleData.canopyOpen));
        aircraftHandleData.gearHandlePosition ? ui->gearLineEdit->setText(tr("Down")) : ui->gearLineEdit->setText(tr("Up"));
        ui->brakeLeftLineEdit->setText(d->unit.formatPosition(aircraftHandleData.brakeLeftPosition));
        ui->brakeRightLineEdit->setText(d->unit.formatPosition(aircraftHandleData.brakeRightPosition));
        ui->steerInputControlLineEdit->setText(d->unit.formatPosition(aircraftHandleData.steerInputControl));
        ui->waterRudderLineEdit->setText(d->unit.formatPosition(aircraftHandleData.waterRudderHandlePosition));        
        aircraftHandleData.tailhookHandlePosition ? ui->tailhookHandleLineEdit->setText(tr("Extended")) : ui->tailhookHandleLineEdit->setText(tr("Retracted"));
        ui->tailhookPositionLineEdit->setText(d->unit.formatPercent(aircraftHandleData.tailhookPosition));
        aircraftHandleData.foldingWingHandlePosition ? ui->wingFoldingHandleLineEdit->setText(tr("Retracted")) : ui->wingFoldingHandleLineEdit->setText(tr("Extended"));
        ui->leftWingFoldingLineEdit->setText(d->unit.formatPercent(aircraftHandleData.leftWingFolding));
        ui->rightWingFoldingLineEdit->setText(d->unit.formatPercent(aircraftHandleData.rightWingFolding));

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const auto css{QStringLiteral("color: %1;").arg(colorName)};
    ui->canopyOpenLineEdit->setStyleSheet(css);
    ui->gearLineEdit->setStyleSheet(css);
    ui->brakeLeftLineEdit->setStyleSheet(css);
    ui->brakeRightLineEdit->setStyleSheet(css);
    ui->steerInputControlLineEdit->setStyleSheet(css);
    ui->waterRudderLineEdit->setStyleSheet(css);    
    ui->tailhookHandleLineEdit->setStyleSheet(css);
    ui->tailhookPositionLineEdit->setStyleSheet(css);
    ui->wingFoldingHandleLineEdit->setStyleSheet(css);
    ui->leftWingFoldingLineEdit->setStyleSheet(css);
    ui->rightWingFoldingLineEdit->setStyleSheet(css);
}
