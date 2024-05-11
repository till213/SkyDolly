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
#include <utility>

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
#include <Model/Attitude.h>
#include <Model/AttitudeData.h>
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
    const auto positionAndAttitude = getCurrentPositionData(timestamp, access);
    QString positionColorName;
    QString attitudeColorName;

    const auto &position = positionAndAttitude.first;
    const auto &attitude = positionAndAttitude.second;
    if (!position.isNull()) {
        // Position
        ui->latitudeLineEdit->setText(d->unit.formatCoordinate(position.latitude) % " (" % d->unit.formatLatitudeDMS(position.latitude) % ")");
        ui->longitudeLineEdit->setText(d->unit.formatCoordinate(position.longitude) % " (" % d->unit.formatLongitudeDMS(position.longitude) % ")");
        ui->altitudeLineEdit->setText(d->unit.formatFeet(position.altitude));
        ui->indicatedAltitudeLineEdit->setText(d->unit.formatFeet(position.indicatedAltitude));

        positionColorName = d->ActiveTextColor.name();
    } else {
        positionColorName = d->DisabledTextColor.name();
    }

    if (!attitude.isNull()) {
        ui->pitchLineEdit->setText(d->unit.formatDegrees(attitude.pitch));
        ui->bankLineEdit->setText(d->unit.formatDegrees(attitude.bank));
        ui->headingLineEdit->setText(d->unit.formatDegrees(attitude.trueHeading));

        // Velocity
        double speedFeetPerSec = attitude.velocityBodyX;
        double speedKnots = Convert::feetPerSecondToKnots(speedFeetPerSec);
        ui->velocityXLineEdit->setText(d->unit.formatKnots(speedKnots) % " (" % d->unit.formatSpeedInFeetPerSecond(speedFeetPerSec) % ")");
        speedFeetPerSec = attitude.velocityBodyY;
        speedKnots = Convert::feetPerSecondToKnots(speedFeetPerSec);
        ui->velocityYLineEdit->setText(d->unit.formatKnots(speedKnots) % " (" % d->unit.formatSpeedInFeetPerSecond(speedFeetPerSec) % ")");
        speedFeetPerSec = attitude.velocityBodyZ;
        speedKnots = Convert::feetPerSecondToKnots(speedFeetPerSec);
        ui->velocityZLineEdit->setText(d->unit.formatKnots(speedKnots) % " (" % d->unit.formatSpeedInFeetPerSecond(speedFeetPerSec) % ")");

        attitudeColorName = d->ActiveTextColor.name();
    } else {
        attitudeColorName = d->DisabledTextColor.name();
    }

    const QString positionCss {QStringLiteral("color: %1;").arg(positionColorName)};
    ui->latitudeLineEdit->setStyleSheet(positionCss);
    ui->longitudeLineEdit->setStyleSheet(positionCss);
    ui->altitudeLineEdit->setStyleSheet(positionCss);
    ui->indicatedAltitudeLineEdit->setStyleSheet(positionCss);

    const QString attitudeCss {QStringLiteral("color: %1;").arg(attitudeColorName)};
    ui->pitchLineEdit->setStyleSheet(attitudeCss);
    ui->bankLineEdit->setStyleSheet(attitudeCss);
    ui->headingLineEdit->setStyleSheet(attitudeCss);
    ui->headingLineEdit->setStyleSheet(attitudeCss);
    ui->velocityXLineEdit->setStyleSheet(attitudeCss);
    ui->velocityYLineEdit->setStyleSheet(attitudeCss);
    ui->velocityZLineEdit->setStyleSheet(attitudeCss);
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

std::pair<PositionData, AttitudeData> AircraftWidget::getCurrentPositionData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    PositionData positionData;
    AttitudeData attitudeData;
    const auto &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const auto &position = aircraft.getPosition();
    const auto &attitude = aircraft.getAttitude();
    const auto &skyConnectManager = SkyConnectManager::getInstance();

    // Position
    if (skyConnectManager.getState() == Connect::State::Recording) {
        if (position.count() > 0) {
            positionData = position.getLast();
        }
    } else {
        const auto t = timestamp != TimeVariableData::InvalidTime ? timestamp : skyConnectManager.getCurrentTimestamp();
        positionData = position.interpolate(t, access);
    };

    // Attitude
    if (skyConnectManager.getState() == Connect::State::Recording) {
        if (attitude.count() > 0) {
            attitudeData = attitude.getLast();
        }
    } else {
        const auto t = timestamp != TimeVariableData::InvalidTime ? timestamp : skyConnectManager.getCurrentTimestamp();
        attitudeData = attitude.interpolate(t, access);
    };

    return std::make_pair(positionData, attitudeData);
}
