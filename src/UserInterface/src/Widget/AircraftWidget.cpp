/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <QStringBuilder>

#include "../../../Kernel/src/Unit.h"
#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/Position.h"
#include "../../../Model/src/PositionData.h"
#include "../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "AircraftWidget.h"
#include "ui_AircraftWidget.h"

class AircraftWidgetPrivate
{
public:
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
    d(std::make_unique<AircraftWidgetPrivate>(*this)),
    ui(std::make_unique<Ui::AircraftWidget>())
{
    ui->setupUi(this);
    initUi();
}

AircraftWidget::~AircraftWidget() noexcept
{}

// PROTECTED SLOTS

void AircraftWidget::updateUi(int64_t timestamp, TimeVariableData::Access access) noexcept
{
    const PositionData &positionData = getCurrentPositionData(timestamp, access);
    QString colorName;

    if (!positionData.isNull()) {
        // Position
        ui->latitudeLineEdit->setText(d->unit.formatLatitude(positionData.latitude));
        ui->longitudeLineEdit->setText(d->unit.formatLongitude(positionData.longitude));
        ui->altitudeLineEdit->setText(d->unit.formatFeet(positionData.altitude));
        ui->pitchLineEdit->setText(d->unit.formatDegrees(positionData.pitch));
        ui->bankLineEdit->setText(d->unit.formatDegrees(positionData.bank));
        ui->headingLineEdit->setText(d->unit.formatDegrees(positionData.heading));

        // Velocity
        double velocityFeetPerSec = positionData.velocityBodyX;
        double velocityKnots = Convert::feetPerSecondToKnots(velocityFeetPerSec);
        ui->velocityXLineEdit->setText(d->unit.formatKnots(velocityKnots) % " (" % d->unit.formatVelocityInFeetPerSecond(velocityFeetPerSec) % ")");
        velocityFeetPerSec = positionData.velocityBodyY;
        velocityKnots = Convert::feetPerSecondToKnots(velocityFeetPerSec);
        ui->velocityYLineEdit->setText(d->unit.formatKnots(velocityKnots) % " (" % d->unit.formatVelocityInFeetPerSecond(velocityFeetPerSec) % ")");
        velocityFeetPerSec = positionData.velocityBodyZ;
        velocityKnots = Convert::feetPerSecondToKnots(velocityFeetPerSec);
        ui->velocityZLineEdit->setText(d->unit.formatKnots(velocityKnots) % " (" % d->unit.formatVelocityInFeetPerSecond(velocityFeetPerSec) % ")");
        ui->rotationVelocityXLineEdit->setText(d->unit.formatVelocityInRadians(positionData.rotationVelocityBodyX));
        ui->rotationVelocityYLineEdit->setText(d->unit.formatVelocityInRadians(positionData.rotationVelocityBodyY));
        ui->rotationVelocityZLineEdit->setText(d->unit.formatVelocityInRadians(positionData.rotationVelocityBodyZ));

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QString("color: %1;").arg(colorName)};
    ui->latitudeLineEdit->setStyleSheet(css);
    ui->longitudeLineEdit->setStyleSheet(css);
    ui->altitudeLineEdit->setStyleSheet(css);
    ui->pitchLineEdit->setStyleSheet(css);
    ui->bankLineEdit->setStyleSheet(css);
    ui->headingLineEdit->setStyleSheet(css);
    ui->headingLineEdit->setStyleSheet(css);
    ui->velocityXLineEdit->setStyleSheet(css);
    ui->velocityYLineEdit->setStyleSheet(css);
    ui->velocityZLineEdit->setStyleSheet(css);
    ui->rotationVelocityXLineEdit->setStyleSheet(css);
    ui->rotationVelocityYLineEdit->setStyleSheet(css);
    ui->rotationVelocityZLineEdit->setStyleSheet(css);
}

// PRIVATE

void AircraftWidget::initUi() noexcept
{
    // Position
    ui->latitudeLineEdit->setToolTip(SimVar::Latitude);
    ui->longitudeLineEdit->setToolTip(SimVar::Longitude);
    ui->altitudeLineEdit->setToolTip(SimVar::Altitude);
    ui->pitchLineEdit->setToolTip(SimVar::Pitch);
    ui->bankLineEdit->setToolTip(SimVar::Bank);
    ui->headingLineEdit->setToolTip(SimVar::Heading);

    // Velocity
    ui->velocityXLineEdit->setToolTip(SimVar::VelocityBodyX);
    ui->velocityYLineEdit->setToolTip(SimVar::VelocityBodyY);
    ui->velocityZLineEdit->setToolTip(SimVar::VelocityBodyZ);
    ui->rotationVelocityXLineEdit->setToolTip(SimVar::RotationVelocityBodyX);
    ui->rotationVelocityYLineEdit->setToolTip(SimVar::RotationVelocityBodyY);
    ui->rotationVelocityZLineEdit->setToolTip(SimVar::RotationVelocityBodyZ);
}

const PositionData &AircraftWidget::getCurrentPositionData(int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().getState() == Connect::State::Recording) {
            return aircraft.getPositionConst().getLast();
        } else {
            if (timestamp != TimeVariableData::InvalidTime) {
                return aircraft.getPositionConst().interpolate(timestamp, access);
            } else {
                return aircraft.getPositionConst().interpolate(skyConnect->get().getCurrentTimestamp(), access);
            }
        };
    } else {
        return PositionData::NullData;
    }
}
