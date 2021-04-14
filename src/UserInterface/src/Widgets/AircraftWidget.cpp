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

#include <QDialog>

#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/World.h"
#include "../../../Model/src/Scenario.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/AircraftData.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "AircraftWidget.h"
#include "ui_AircraftWidget.h"

class AircraftWidgetPrivate
{
public:
    AircraftWidgetPrivate(const QWidget &widget, SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect),
          ActiveTextColor(widget.palette().color(QPalette::Active, QPalette::WindowText)),
          DisabledTextColor(widget.palette().color(QPalette::Disabled, QPalette::WindowText))
    {}

    SkyConnectIntf &skyConnect;
    const QColor ActiveTextColor;
    const QColor DisabledTextColor;
};

AircraftWidget::AircraftWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QWidget(parent),
    d(std::make_unique<AircraftWidgetPrivate>(*this, skyConnect)),
    ui(std::make_unique<Ui::AircraftWidget>())
{
    ui->setupUi(this);
    initUi();
}

AircraftWidget::~AircraftWidget()
{
}

// PROTECTED

void AircraftWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi(d->skyConnect.getCurrentTimestamp(), TimeVariableData::Access::Seek);
    connect(&d->skyConnect, &SkyConnectIntf::timestampChanged,
            this, &AircraftWidget::handleTimestampChanged);
}

void AircraftWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    disconnect(&d->skyConnect, &SkyConnectIntf::timestampChanged,
            this, &AircraftWidget::handleTimestampChanged);
}

// PRIVATE

void AircraftWidget::initUi()
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

    // Acceleration
    ui->accelerationXLineEdit->setToolTip(SimVar::AccelerationBodyX);
    ui->accelerationYLineEdit->setToolTip(SimVar::AccelerationBodyY);
    ui->accelerationZLineEdit->setToolTip(SimVar::AccelerationBodyZ);
    ui->rotationAccelerationXLineEdit->setToolTip(SimVar::RotationAccelerationBodyX);
    ui->rotationAccelerationYLineEdit->setToolTip(SimVar::RotationAccelerationBodyY);
    ui->rotationAccelerationZLineEdit->setToolTip(SimVar::RotationAccelerationBodyZ);
}

void AircraftWidget::updateUi(qint64 timestamp, TimeVariableData::Access access)
{
    const AircraftData &aircraftData = getCurrentAircraftData(timestamp, access);
    QString colorName;

    if (!aircraftData.isNull()) {
        // Position
        ui->latitudeLineEdit->setText(QString::number(aircraftData.latitude));
        ui->longitudeLineEdit->setText(QString::number(aircraftData.longitude));
        ui->altitudeLineEdit->setText(QString::number(aircraftData.altitude));
        ui->pitchLineEdit->setText(QString::number(aircraftData.pitch));
        ui->bankLineEdit->setText(QString::number(aircraftData.bank));
        ui->headingLineEdit->setText(QString::number(aircraftData.heading));

        // Velocity
        ui->velocityXLineEdit->setText(QString::number(aircraftData.velocityBodyX));
        ui->velocityYLineEdit->setText(QString::number(aircraftData.velocityBodyY));
        ui->velocityZLineEdit->setText(QString::number(aircraftData.velocityBodyZ));
        ui->rotationVelocityXLineEdit->setText(QString::number(aircraftData.rotationVelocityBodyX));
        ui->rotationVelocityYLineEdit->setText(QString::number(aircraftData.rotationVelocityBodyY));
        ui->rotationVelocityZLineEdit->setText(QString::number(aircraftData.rotationVelocityBodyZ));

        // Acceleration
        ui->accelerationXLineEdit->setText(QString::number(aircraftData.accelerationBodyX));
        ui->accelerationYLineEdit->setText(QString::number(aircraftData.accelerationBodyY));
        ui->accelerationZLineEdit->setText(QString::number(aircraftData.accelerationBodyZ));
        ui->rotationAccelerationXLineEdit->setText(QString::number(aircraftData.rotationAccelerationBodyX));
        ui->rotationAccelerationYLineEdit->setText(QString::number(aircraftData.rotationAccelerationBodyY));
        ui->rotationAccelerationZLineEdit->setText(QString::number(aircraftData.rotationAccelerationBodyZ));

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
    ui->accelerationXLineEdit->setStyleSheet(css);
    ui->accelerationYLineEdit->setStyleSheet(css);
    ui->accelerationZLineEdit->setStyleSheet(css);
    ui->rotationAccelerationXLineEdit->setStyleSheet(css);
    ui->rotationAccelerationYLineEdit->setStyleSheet(css);
    ui->rotationAccelerationZLineEdit->setStyleSheet(css);
}

const AircraftData &AircraftWidget::getCurrentAircraftData(qint64 timestamp, TimeVariableData::Access access) const
{
    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();

    if (d->skyConnect.getState() == Connect::State::Recording) {
        return aircraft.getLast();
    } else {
        if (timestamp != TimeVariableData::InvalidTime) {
            return aircraft.interpolate(timestamp, access);
        } else {
            return aircraft.interpolate(d->skyConnect.getCurrentTimestamp(), access);
        }
    };
}

// PRIVATE SLOTS

void AircraftWidget::handleTimestampChanged(qint64 timestamp, TimeVariableData::Access access)
{
    updateUi(timestamp, access);
}

