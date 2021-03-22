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
#include "LightVariablesWidget.h"
#include "ui_LightVariablesWidget.h"

class LightVariablesWidgetPrivate
{
public:
    LightVariablesWidgetPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
};

LightVariablesWidget::LightVariablesWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QWidget(parent),
    d(std::make_unique<LightVariablesWidgetPrivate>(skyConnect)),
    ui(std::make_unique<Ui::LightVariablesWidget>())
{
    ui->setupUi(this);
    initUi();
}

LightVariablesWidget::~LightVariablesWidget()
{
}

// PROTECTED

void LightVariablesWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi();

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    // Signal sent while recording
    connect(&aircraft, &Aircraft::dataChanged,
            this, &LightVariablesWidget::updateLightDataUi);
    // Signal sent while playing
    connect(&d->skyConnect, &SkyConnectIntf::currentTimestampChanged,
            this, &LightVariablesWidget::updateLightDataUi);
}

void LightVariablesWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    disconnect(&aircraft, &Aircraft::dataChanged,
               this, &LightVariablesWidget::updateLightDataUi);
    disconnect(&d->skyConnect, &SkyConnectIntf::currentTimestampChanged,
            this, &LightVariablesWidget::updateLightDataUi);
}

// PRIVATE

void LightVariablesWidget::initUi()
{
    ui->lightStateLineEdit->setToolTip(Const::LightStates);

    // Make the light state checkboxes checkable, but not for the user
    ui->navigationCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->navigationCheckBox->setFocusPolicy(Qt::NoFocus);

    ui->beaconCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->beaconCheckBox->setFocusPolicy(Qt::NoFocus);

    ui->landingCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->landingCheckBox->setFocusPolicy(Qt::NoFocus);

    ui->taxiCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->taxiCheckBox->setFocusPolicy(Qt::NoFocus);

    ui->strobeCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->strobeCheckBox->setFocusPolicy(Qt::NoFocus);

    ui->panelCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->panelCheckBox->setFocusPolicy(Qt::NoFocus);

    ui->recognitionCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->recognitionCheckBox->setFocusPolicy(Qt::NoFocus);

    ui->wingCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->wingCheckBox->setFocusPolicy(Qt::NoFocus);

    ui->logoCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->logoCheckBox->setFocusPolicy(Qt::NoFocus);

    ui->cabinCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->cabinCheckBox->setFocusPolicy(Qt::NoFocus);
}

void LightVariablesWidget::updateUi()
{
    updateLightDataUi();
}

const AircraftData &LightVariablesWidget::getCurrentAircraftData() const
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

void LightVariablesWidget::updateLightDataUi()
{
    const AircraftData &aircraftData = getCurrentAircraftData();

    // Lights
    ui->lightStateLineEdit->setText(QString::number(aircraftData.lightStates));

    ui->navigationCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Navigation));
    ui->beaconCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Beacon));
    ui->landingCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Landing));
    ui->taxiCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Taxi));
    ui->strobeCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Strobe));
    ui->panelCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Panel));
    ui->recognitionCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Recognition));
    ui->wingCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Wing));
    ui->logoCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Logo));
    ui->cabinCheckBox->setChecked(aircraftData.lightStates.testFlag(SimTypes::LightState::Cabin));
}
