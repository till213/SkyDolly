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
#include <QDialog>
#include <QColor>
#include <QPalette>

#include <Model/SimVar.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Light.h>
#include <Model/LightData.h>
#include <Model/TimeVariableData.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/Connect.h>
#include "LightWidget.h"
#include "ui_LightWidget.h"

struct LightWidgetPrivate
{
    LightWidgetPrivate(const QWidget &widget) noexcept
        : ActiveTextColor(widget.palette().color(QPalette::Active, QPalette::WindowText)),
          DisabledTextColor(widget.palette().color(QPalette::Disabled, QPalette::WindowText))
    {}

    const QColor ActiveTextColor;
    const QColor DisabledTextColor;
};

// PUBLIC

LightWidget::LightWidget(QWidget *parent) noexcept :
    AbstractSimulationVariableWidget(parent),
    ui(std::make_unique<Ui::LightWidget>()),
    d(std::make_unique<LightWidgetPrivate>(*this))
{
    ui->setupUi(this);
    initUi();
}

LightWidget::~LightWidget() noexcept
{}

// PROTECTED


// PRIVATE

void LightWidget::initUi() noexcept
{
    ui->lightStateLineEdit->setToolTip(SimVar::LightStates);

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

LightData LightWidget::getCurrentLightData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    LightData lightData;
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().getState() == Connect::State::Recording) {
            return aircraft.getLight().getLast();
        } else {
            if (timestamp != TimeVariableData::InvalidTime) {
                lightData = aircraft.getLight().interpolate(timestamp, access);
            } else {
                lightData = aircraft.getLight().interpolate(skyConnect->get().getCurrentTimestamp(), access);
            }
        };
    }
    return lightData;
}

// PRIVATE SLOTS

void LightWidget::updateUi(std::int64_t timestamp, TimeVariableData::Access access) noexcept
{
    const LightData &lightData = getCurrentLightData(timestamp, access);
    QString colorName;

    if (!lightData.isNull()) {
        ui->lightStateLineEdit->setText(QString::number(lightData.lightStates));

        ui->navigationCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Navigation));
        ui->beaconCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Beacon));
        ui->landingCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Landing));
        ui->taxiCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Taxi));
        ui->strobeCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Strobe));
        ui->panelCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Panel));
        ui->recognitionCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Recognition));
        ui->wingCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Wing));
        ui->logoCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Logo));
        ui->cabinCheckBox->setChecked(lightData.lightStates.testFlag(SimType::LightState::Cabin));
        colorName = d->ActiveTextColor.name();

    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QString("color: %1;").arg(colorName)};
    ui->lightStateLineEdit->setStyleSheet(css);
    ui->navigationCheckBox->setStyleSheet(css);
    ui->beaconCheckBox->setStyleSheet(css);
    ui->landingCheckBox->setStyleSheet(css);
    ui->taxiCheckBox->setStyleSheet(css);
    ui->strobeCheckBox->setStyleSheet(css);
    ui->panelCheckBox->setStyleSheet(css);
    ui->recognitionCheckBox->setStyleSheet(css);
    ui->wingCheckBox->setStyleSheet(css);
    ui->logoCheckBox->setStyleSheet(css);
    ui->cabinCheckBox->setStyleSheet(css);
}
