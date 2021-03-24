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
#include "../../../Model/src/EngineData.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "EngineWidget.h"
#include "ui_EngineWidget.h"

class EngineWidgetPrivate
{
public:
    EngineWidgetPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
};

EngineWidget::EngineWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QWidget(parent),
    d(std::make_unique<EngineWidgetPrivate>(skyConnect)),
    ui(std::make_unique<Ui::EngineWidget>())
{
    ui->setupUi(this);
    initUi();
}

EngineWidget::~EngineWidget()
{
}

// PROTECTED

void EngineWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi();

    const Engine &engine = World::getInstance().getCurrentScenario().getUserAircraft().getEngineConst();
    // Signal sent while recording
    connect(&engine, &Engine::dataChanged,
            this, &EngineWidget::updateDataUi);
    // Signal sent while playing
    connect(&d->skyConnect, &SkyConnectIntf::currentTimestampChanged,
            this, &EngineWidget::updateDataUi);
}

void EngineWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const Engine &engine = World::getInstance().getCurrentScenario().getUserAircraft().getEngineConst();
    disconnect(&engine, &Engine::dataChanged,
               this, &EngineWidget::updateDataUi);
    disconnect(&d->skyConnect, &SkyConnectIntf::currentTimestampChanged,
            this, &EngineWidget::updateDataUi);
}

// PRIVATE

void EngineWidget::initUi()
{
    ui->throttle1LineEdit->setToolTip(SimVar::ThrottleLeverPosition1);
    ui->throttle2LineEdit->setToolTip(SimVar::ThrottleLeverPosition2);
    ui->throttle3LineEdit->setToolTip(SimVar::ThrottleLeverPosition3);
    ui->throttle4LineEdit->setToolTip(SimVar::ThrottleLeverPosition4);
    ui->propeller1LineEdit->setToolTip(SimVar::PropellerLeverPosition1);
    ui->propeller2LineEdit->setToolTip(SimVar::PropellerLeverPosition2);
    ui->propeller3LineEdit->setToolTip(SimVar::PropellerLeverPosition3);
    ui->propeller4LineEdit->setToolTip(SimVar::PropellerLeverPosition4);
    ui->mixture1LineEdit->setToolTip(SimVar::MixtureLeverPosition1);
    ui->mixture2LineEdit->setToolTip(SimVar::MixtureLeverPosition2);
    ui->mixture3LineEdit->setToolTip(SimVar::MixtureLeverPosition3);
    ui->mixture4LineEdit->setToolTip(SimVar::MixtureLeverPosition4);
}

void EngineWidget::updateUi()
{
    updateDataUi();
}

const EngineData &EngineWidget::getCurrentEngineData() const
{
    const AircraftData aircraftData;
    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();

    if (d->skyConnect.getState() == Connect::State::Recording) {
        return aircraft.getEngineConst().getLastEngineData();
    } else {
        return aircraft.getEngineConst().interpolateEngineData(d->skyConnect.getCurrentTimestamp());
    };
}

// PRIVATE SLOTS

void EngineWidget::updateDataUi()
{
    const EngineData &engineData = getCurrentEngineData();

    // General engine
    if (!engineData.isNull()) {
        ui->throttle1LineEdit->setText(QString::number(engineData.throttleLeverPosition1));
        ui->throttle2LineEdit->setText(QString::number(engineData.throttleLeverPosition2));
        ui->throttle3LineEdit->setText(QString::number(engineData.throttleLeverPosition3));
        ui->throttle4LineEdit->setText(QString::number(engineData.throttleLeverPosition4));
        ui->propeller1LineEdit->setText(QString::number(engineData.propellerLeverPosition1));
        ui->propeller2LineEdit->setText(QString::number(engineData.propellerLeverPosition2));
        ui->propeller3LineEdit->setText(QString::number(engineData.propellerLeverPosition3));
        ui->propeller4LineEdit->setText(QString::number(engineData.propellerLeverPosition4));
        ui->mixture1LineEdit->setText(QString::number(engineData.mixtureLeverPosition1));
        ui->mixture2LineEdit->setText(QString::number(engineData.mixtureLeverPosition2));
        ui->mixture3LineEdit->setText(QString::number(engineData.mixtureLeverPosition3));
        ui->mixture4LineEdit->setText(QString::number(engineData.mixtureLeverPosition4));
    }
}
