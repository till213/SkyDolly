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

#include "../../../Kernel/src/Const.h"
#include "../../../Kernel/src/Aircraft.h"
#include "../../../Kernel/src/AircraftInfo.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "AircraftVariablesWidget.h"
#include "ui_AircraftVariablesWidget.h"

class AircraftVariablesWidgetPrivate
{
public:
    AircraftVariablesWidgetPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
};

AircraftVariablesWidget::AircraftVariablesWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QWidget(parent),
    d(std::make_unique<AircraftVariablesWidgetPrivate>(skyConnect)),
    ui(std::make_unique<Ui::AircraftVariablesWidget>())
{
    ui->setupUi(this);
    initUi();
}

AircraftVariablesWidget::~AircraftVariablesWidget()
{
}

// PROTECTED

void AircraftVariablesWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi();

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    // Signal sent while recording
    connect(&aircraft, &Aircraft::dataChanged,
            this, &AircraftVariablesWidget::updateAircraftDataUi);
    // Signal sent while playing
    connect(&d->skyConnect, &SkyConnectIntf::aircraftDataSent,
            this, &AircraftVariablesWidget::updateAircraftDataUi);
}

void AircraftVariablesWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    disconnect(&aircraft, &Aircraft::dataChanged,
               this, &AircraftVariablesWidget::updateAircraftDataUi);
    disconnect(&d->skyConnect, &SkyConnectIntf::aircraftDataSent,
            this, &AircraftVariablesWidget::updateAircraftDataUi);
}

// PRIVATE

void AircraftVariablesWidget::initUi()
{
    ui->latitudeLineEdit->setToolTip(Const::Latitude);
    ui->longitudeLineEdit->setToolTip(Const::Longitude);
    ui->altitudeLineEdit->setToolTip(Const::Altitude);
    ui->pitchLineEdit->setToolTip(Const::Pitch);
    ui->bankLineEdit->setToolTip(Const::Bank);
    ui->headingLineEdit->setToolTip(Const::Heading);

    ui->velocityXLineEdit->setToolTip(Const::VelocityBodyX);
    ui->velocityYLineEdit->setToolTip(Const::VelocityBodyY);
    ui->velocityZLineEdit->setToolTip(Const::VelocityBodyZ);
    ui->rotationVelocityXLineEdit->setToolTip(Const::RotationVelocityBodyX);
    ui->rotationVelocityYLineEdit->setToolTip(Const::RotationVelocityBodyY);
    ui->rotationVelocityZLineEdit->setToolTip(Const::RotationVelocityBodyZ);
}

void AircraftVariablesWidget::updateUi()
{
    updateAircraftDataUi();
}

const AircraftData &AircraftVariablesWidget::getCurrentAircraftData() const
{
    const AircraftData aircraftData;
    const Aircraft &aircraft = d->skyConnect.getAircraft();

    if (d->skyConnect.getState() == Connect::State::Recording) {
        return aircraft.getLastAircraftData();
    } else {
        return d->skyConnect.getCurrentAircraftData();
    };
}

// PRIVATE SLOTS

void AircraftVariablesWidget::updateAircraftDataUi()
{
    const AircraftData &aircraftData = getCurrentAircraftData();

    // Aircraft position
    ui->latitudeLineEdit->setText(QString::number(aircraftData.latitude));
    ui->longitudeLineEdit->setText(QString::number(aircraftData.longitude));
    ui->altitudeLineEdit->setText(QString::number(aircraftData.altitude));
    ui->pitchLineEdit->setText(QString::number(aircraftData.pitch));
    ui->bankLineEdit->setText(QString::number(aircraftData.bank));
    ui->headingLineEdit->setText(QString::number(aircraftData.heading));

    // Forces
    ui->velocityXLineEdit->setText(QString::number(aircraftData.velocityBodyX));
    ui->velocityYLineEdit->setText(QString::number(aircraftData.velocityBodyY));
    ui->velocityZLineEdit->setText(QString::number(aircraftData.velocityBodyZ));
    ui->rotationVelocityXLineEdit->setText(QString::number(aircraftData.rotationVelocityBodyX));
    ui->rotationVelocityYLineEdit->setText(QString::number(aircraftData.rotationVelocityBodyY));
    ui->rotationVelocityZLineEdit->setText(QString::number(aircraftData.rotationVelocityBodyZ));
}
