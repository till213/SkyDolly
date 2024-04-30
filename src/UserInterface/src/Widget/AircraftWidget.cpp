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

#include <QDialog>
#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>

#include <Kernel/Unit.h>
#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Connect/Connect.h>
#include "AircraftWidget.h"
#include "ui_AircraftWidget.h"

struct AircraftWidgetPrivate
{
    AircraftWidgetPrivate(const QWidget &widget) noexcept
        : ActiveTextColor(widget.palette().color(QPalette::Active, QPalette::WindowText)),
          DisabledTextColor(widget.palette().color(QPalette::Disabled, QPalette::WindowText))
    {}

    Unit unit;
    const QColor ActiveTextColor;
    const QColor DisabledTextColor;
};

// PUBLIC

AircraftWidget::AircraftWidget(QWidget *parent) noexcept :
    AbstractSimulationVariableWidget(parent),
    ui(std::make_unique<Ui::AircraftWidget>()),
    d(std::make_unique<AircraftWidgetPrivate>(*this))
{
    ui->setupUi(this);
    initUi();
}

AircraftWidget::~AircraftWidget() = default;

// PROTECTED SLOTS

void AircraftWidget::updateUi(std::int64_t timestamp, TimeVariableData::Access access) noexcept
{
    const PositionData &positionData = getCurrentPositionData(timestamp, access);
    QString colorName;

    if (!positionData.isNull()) {
        // Position
        ui->latitudeLineEdit->setText(d->unit.formatCoordinate(positionData.latitude) % " (" % d->unit.formatLatitudeDMS(positionData.latitude) % ")");
        ui->longitudeLineEdit->setText(d->unit.formatCoordinate(positionData.longitude) % " (" % d->unit.formatLongitudeDMS(positionData.longitude) % ")");
        ui->altitudeLineEdit->setText(d->unit.formatFeet(positionData.altitude));
        ui->indicatedAltitudeLineEdit->setText(d->unit.formatFeet(positionData.indicatedAltitude));
        ui->pitchLineEdit->setText(d->unit.formatDegrees(positionData.pitch));
        ui->bankLineEdit->setText(d->unit.formatDegrees(positionData.bank));
        ui->headingLineEdit->setText(d->unit.formatDegrees(positionData.trueHeading));

        // Velocity
        double speedFeetPerSec = positionData.velocityBodyX;
        double speedKnots = Convert::feetPerSecondToKnots(speedFeetPerSec);
        ui->velocityXLineEdit->setText(d->unit.formatKnots(speedKnots) % " (" % d->unit.formatSpeedInFeetPerSecond(speedFeetPerSec) % ")");
        speedFeetPerSec = positionData.velocityBodyY;
        speedKnots = Convert::feetPerSecondToKnots(speedFeetPerSec);
        ui->velocityYLineEdit->setText(d->unit.formatKnots(speedKnots) % " (" % d->unit.formatSpeedInFeetPerSecond(speedFeetPerSec) % ")");
        speedFeetPerSec = positionData.velocityBodyZ;
        speedKnots = Convert::feetPerSecondToKnots(speedFeetPerSec);
        ui->velocityZLineEdit->setText(d->unit.formatKnots(speedKnots) % " (" % d->unit.formatSpeedInFeetPerSecond(speedFeetPerSec) % ")");

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QStringLiteral("color: %1;").arg(colorName)};
    ui->latitudeLineEdit->setStyleSheet(css);
    ui->longitudeLineEdit->setStyleSheet(css);
    ui->altitudeLineEdit->setStyleSheet(css);
    ui->indicatedAltitudeLineEdit->setStyleSheet(css);
    ui->pitchLineEdit->setStyleSheet(css);
    ui->bankLineEdit->setStyleSheet(css);
    ui->headingLineEdit->setStyleSheet(css);
    ui->headingLineEdit->setStyleSheet(css);
    ui->velocityXLineEdit->setStyleSheet(css);
    ui->velocityYLineEdit->setStyleSheet(css);
    ui->velocityZLineEdit->setStyleSheet(css);
}

// PRIVATE

void AircraftWidget::initUi() noexcept
{
    // Position
    ui->latitudeLineEdit->setToolTip(QString::fromLatin1(SimVar::Latitude));
    ui->longitudeLineEdit->setToolTip(QString::fromLatin1(SimVar::Longitude));
    ui->altitudeLineEdit->setToolTip(QString::fromLatin1(SimVar::Altitude));
    ui->indicatedAltitudeLineEdit->setToolTip(QString::fromLatin1(SimVar::IndicatedAltitude));
    ui->pitchLineEdit->setToolTip(QString::fromLatin1(SimVar::Pitch));
    ui->bankLineEdit->setToolTip(QString::fromLatin1(SimVar::Bank));
    ui->headingLineEdit->setToolTip(QString::fromLatin1(SimVar::TrueHeading));

    // Velocity
    ui->velocityXLineEdit->setToolTip(QString::fromLatin1(SimVar::VelocityBodyX));
    ui->velocityYLineEdit->setToolTip(QString::fromLatin1(SimVar::VelocityBodyY));
    ui->velocityZLineEdit->setToolTip(QString::fromLatin1(SimVar::VelocityBodyZ));
}

PositionData AircraftWidget::getCurrentPositionData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    PositionData positionData;
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().getState() == Connect::State::Recording) {
            if (aircraft.getPosition().count() > 0) {
                positionData = aircraft.getPosition().getLast();
            }
        } else {
            if (timestamp != TimeVariableData::InvalidTime) {
                positionData = aircraft.getPosition().interpolate(timestamp, access);
            } else {
                positionData = aircraft.getPosition().interpolate(skyConnect->get().getCurrentTimestamp(), access);
            }
        };
    }
    return positionData;
}
