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
#include <optional>

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
    AbstractSimulationVariableWidget(parent),
    ui(std::make_unique<Ui::AircraftHandleWidget>()),
    d(std::make_unique<AircraftHandleWidgetPrivate>(*this))
{
    ui->setupUi(this);
    initUi();
}

AircraftHandleWidget::~AircraftHandleWidget() = default;

// PRIVATE

void AircraftHandleWidget::initUi() noexcept
{
    ui->gearLineEdit->setToolTip(QString::fromLatin1(SimVar::GearHandlePosition));
    ui->brakeLeftLineEdit->setToolTip(QString::fromLatin1(SimVar::BrakeLeftPosition));
    ui->brakeRightLineEdit->setToolTip(QString::fromLatin1(SimVar::BrakeRightPosition));
    ui->waterRudderLineEdit->setToolTip(QString::fromLatin1(SimVar::WaterRudderHandlePosition));
    ui->smokeEnabledLineEdit->setToolTip(QString::fromLatin1(SimVar::SmokeEnable));
    ui->canopyOpenLineEdit->setToolTip(QString::fromLatin1(SimVar::CanopyOpen));
    ui->tailhookHandleLineEdit->setToolTip(QString::fromLatin1(SimVar::TailhookHandle));
    ui->tailhookPositionLineEdit->setToolTip(QString::fromLatin1(SimVar::TailhookPosition));
    ui->wingFoldingHandleLineEdit->setToolTip(QString::fromLatin1(SimVar::FoldingWingHandlePosition));
    ui->leftWingFoldingLineEdit->setToolTip(QString::fromLatin1(SimVar::FoldingWingLeftPercent));
    ui->rightWingFoldingLineEdit->setToolTip(QString::fromLatin1(SimVar::FoldingWingRightPercent));
}

AircraftHandleData AircraftHandleWidget::getCurrentAircraftHandleData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    AircraftHandleData aircraftHandleData;
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().getState() == Connect::State::Recording) {
            if (aircraft.getAircraftHandle().count() > 0) {
                aircraftHandleData = aircraft.getAircraftHandle().getLast();
            }
        } else {
            if (timestamp != TimeVariableData::InvalidTime) {
                aircraftHandleData = aircraft.getAircraftHandle().interpolate(timestamp, access);
            } else {
                aircraftHandleData = aircraft.getAircraftHandle().interpolate(skyConnect->get().getCurrentTimestamp(), access);
            }
        };
    }
    return aircraftHandleData;
}

// PRIVATE SLOTS

void AircraftHandleWidget::updateUi(std::int64_t timestamp, TimeVariableData::Access access) noexcept
{
    const AircraftHandleData &aircraftHandleData = getCurrentAircraftHandleData(timestamp, access);
    QString colorName;

    if (!aircraftHandleData.isNull()) {
        aircraftHandleData.gearHandlePosition ? ui->gearLineEdit->setText(tr("Down")) : ui->gearLineEdit->setText(tr("Up"));
        ui->brakeLeftLineEdit->setText(d->unit.formatPosition(aircraftHandleData.brakeLeftPosition));
        ui->brakeRightLineEdit->setText(d->unit.formatPosition(aircraftHandleData.brakeRightPosition));
        ui->waterRudderLineEdit->setText(d->unit.formatPosition(aircraftHandleData.waterRudderHandlePosition));
        aircraftHandleData.smokeEnabled ? ui->smokeEnabledLineEdit->setText(tr("On")) : ui->smokeEnabledLineEdit->setText(tr("Off"));
        ui->canopyOpenLineEdit->setText(d->unit.formatPercent(aircraftHandleData.canopyOpen));
        aircraftHandleData.tailhookHandlePosition ? ui->tailhookHandleLineEdit->setText(tr("Extended")) : ui->tailhookHandleLineEdit->setText(tr("Retracted"));
        ui->tailhookPositionLineEdit->setText(d->unit.formatPercent(aircraftHandleData.tailhookPosition));
        aircraftHandleData.foldingWingHandlePosition ? ui->wingFoldingHandleLineEdit->setText(tr("Retracted")) : ui->wingFoldingHandleLineEdit->setText(tr("Extended"));
        ui->leftWingFoldingLineEdit->setText(d->unit.formatPercent(aircraftHandleData.leftWingFolding));
        ui->rightWingFoldingLineEdit->setText(d->unit.formatPercent(aircraftHandleData.rightWingFolding));

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QStringLiteral("color: %1;").arg(colorName)};
    ui->gearLineEdit->setStyleSheet(css);
    ui->brakeLeftLineEdit->setStyleSheet(css);
    ui->brakeRightLineEdit->setStyleSheet(css);
    ui->waterRudderLineEdit->setStyleSheet(css);
    ui->smokeEnabledLineEdit->setStyleSheet(css);
    ui->canopyOpenLineEdit->setStyleSheet(css);
    ui->tailhookHandleLineEdit->setStyleSheet(css);
    ui->tailhookPositionLineEdit->setStyleSheet(css);
    ui->wingFoldingHandleLineEdit->setStyleSheet(css);
    ui->leftWingFoldingLineEdit->setStyleSheet(css);
    ui->rightWingFoldingLineEdit->setStyleSheet(css);
}
