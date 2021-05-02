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

#include <QString>
#include <QDialog>
#include <QColor>
#include <QPalette>

#include "../../../Model/src/SimVar.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/AircraftData.h"
#include "../../../Model/src/EngineData.h"
#include "../../../Model/src/TimeVariableData.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "../Unit.h"
#include "EngineWidget.h"
#include "ui_EngineWidget.h"

class EngineWidgetPrivate
{
public:
    EngineWidgetPrivate(const QWidget &widget, SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect),
          ActiveTextColor(widget.palette().color(QPalette::Active, QPalette::WindowText)),
          DisabledTextColor(widget.palette().color(QPalette::Disabled, QPalette::WindowText))
    {}

    SkyConnectIntf &skyConnect;
    Unit unit;
    const QColor ActiveTextColor;
    const QColor DisabledTextColor;
};

// PUBLIC

EngineWidget::EngineWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QWidget(parent),
    d(std::make_unique<EngineWidgetPrivate>(*this, skyConnect)),
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

    updateUi(d->skyConnect.getCurrentTimestamp(), TimeVariableData::Access::Seek);
    connect(&d->skyConnect, &SkyConnectIntf::timestampChanged,
            this, &EngineWidget::handleTimestampChanged);
}

void EngineWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    disconnect(&d->skyConnect, &SkyConnectIntf::timestampChanged,
            this, &EngineWidget::handleTimestampChanged);
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
    ui->cowlFlaps1LineEdit->setToolTip(SimVar::RecipEngineCowlFlapPosition1);
    ui->cowlFlaps2LineEdit->setToolTip(SimVar::RecipEngineCowlFlapPosition2);
    ui->cowlFlaps3LineEdit->setToolTip(SimVar::RecipEngineCowlFlapPosition3);
    ui->cowlFlaps4LineEdit->setToolTip(SimVar::RecipEngineCowlFlapPosition4);

    // Make the master battery and starter checkboxes checkable, but not for the user
    ui->masterBattery1CheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->masterBattery1CheckBox->setFocusPolicy(Qt::NoFocus);
    ui->masterBattery2CheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->masterBattery2CheckBox->setFocusPolicy(Qt::NoFocus);
    ui->masterBattery3CheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->masterBattery3CheckBox->setFocusPolicy(Qt::NoFocus);
    ui->masterBattery4CheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->masterBattery4CheckBox->setFocusPolicy(Qt::NoFocus);

    ui->generalEngineStarter1CheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->generalEngineStarter1CheckBox->setFocusPolicy(Qt::NoFocus);
    ui->generalEngineStarter2CheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->generalEngineStarter2CheckBox->setFocusPolicy(Qt::NoFocus);
    ui->generalEngineStarter3CheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->generalEngineStarter3CheckBox->setFocusPolicy(Qt::NoFocus);
    ui->generalEngineStarter4CheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->generalEngineStarter4CheckBox->setFocusPolicy(Qt::NoFocus);
}

void EngineWidget::updateUi(qint64 timestamp, TimeVariableData::Access access)
{
    const EngineData &engineData = getCurrentEngineData(timestamp, access);
    QString colorName;

    if (!engineData.isNull()) {
        ui->throttle1LineEdit->setText(d->unit.formatPosition(engineData.throttleLeverPosition1));
        ui->throttle2LineEdit->setText(d->unit.formatPosition(engineData.throttleLeverPosition2));
        ui->throttle3LineEdit->setText(d->unit.formatPosition(engineData.throttleLeverPosition3));
        ui->throttle4LineEdit->setText(d->unit.formatPosition(engineData.throttleLeverPosition4));
        ui->propeller1LineEdit->setText(d->unit.formatPosition(engineData.propellerLeverPosition1));
        ui->propeller2LineEdit->setText(d->unit.formatPosition(engineData.propellerLeverPosition2));
        ui->propeller3LineEdit->setText(d->unit.formatPosition(engineData.propellerLeverPosition3));
        ui->propeller4LineEdit->setText(d->unit.formatPosition(engineData.propellerLeverPosition4));
        ui->mixture1LineEdit->setText(d->unit.formatPercent(engineData.mixtureLeverPosition1));
        ui->mixture2LineEdit->setText(d->unit.formatPercent(engineData.mixtureLeverPosition2));
        ui->mixture3LineEdit->setText(d->unit.formatPercent(engineData.mixtureLeverPosition3));
        ui->mixture4LineEdit->setText(d->unit.formatPercent(engineData.mixtureLeverPosition4));
        ui->cowlFlaps1LineEdit->setText(d->unit.formatPercent(engineData.cowlFlapPosition1));
        ui->cowlFlaps2LineEdit->setText(d->unit.formatPercent(engineData.cowlFlapPosition2));
        ui->cowlFlaps3LineEdit->setText(d->unit.formatPercent(engineData.cowlFlapPosition3));
        ui->cowlFlaps4LineEdit->setText(d->unit.formatPercent(engineData.cowlFlapPosition4));

        ui->masterBattery1CheckBox->setChecked(engineData.electricalMasterBattery1);
        ui->masterBattery2CheckBox->setChecked(engineData.electricalMasterBattery2);
        ui->masterBattery3CheckBox->setChecked(engineData.electricalMasterBattery3);
        ui->masterBattery4CheckBox->setChecked(engineData.electricalMasterBattery4);
        ui->generalEngineStarter1CheckBox->setChecked(engineData.generalEngineStarter1);
        ui->generalEngineStarter2CheckBox->setChecked(engineData.generalEngineStarter2);
        ui->generalEngineStarter3CheckBox->setChecked(engineData.generalEngineStarter3);
        ui->generalEngineStarter4CheckBox->setChecked(engineData.generalEngineStarter4);

        colorName = d->ActiveTextColor.name();
    } else {
        colorName = d->DisabledTextColor.name();
    }

    const QString css{QString("color: %1;").arg(colorName)};
    ui->throttle1LineEdit->setStyleSheet(css);
    ui->throttle2LineEdit->setStyleSheet(css);
    ui->throttle3LineEdit->setStyleSheet(css);
    ui->throttle4LineEdit->setStyleSheet(css);
    ui->propeller1LineEdit->setStyleSheet(css);
    ui->propeller2LineEdit->setStyleSheet(css);
    ui->propeller3LineEdit->setStyleSheet(css);
    ui->propeller4LineEdit->setStyleSheet(css);
    ui->mixture1LineEdit->setStyleSheet(css);
    ui->mixture2LineEdit->setStyleSheet(css);
    ui->mixture3LineEdit->setStyleSheet(css);
    ui->mixture4LineEdit->setStyleSheet(css);
    ui->cowlFlaps1LineEdit->setStyleSheet(css);
    ui->cowlFlaps2LineEdit->setStyleSheet(css);
    ui->cowlFlaps3LineEdit->setStyleSheet(css);
    ui->cowlFlaps4LineEdit->setStyleSheet(css);
    ui->masterBattery1CheckBox->setStyleSheet(css);
    ui->masterBattery2CheckBox->setStyleSheet(css);
    ui->masterBattery3CheckBox->setStyleSheet(css);
    ui->masterBattery4CheckBox->setStyleSheet(css);
    ui->generalEngineStarter1CheckBox->setStyleSheet(css);
    ui->generalEngineStarter2CheckBox->setStyleSheet(css);
    ui->generalEngineStarter3CheckBox->setStyleSheet(css);
    ui->generalEngineStarter4CheckBox->setStyleSheet(css);
}

const EngineData &EngineWidget::getCurrentEngineData(qint64 timestamp, TimeVariableData::Access access) const
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    if (d->skyConnect.getState() == Connect::State::Recording) {
        return aircraft.getEngineConst().getLast();
    } else {
        if (timestamp != TimeVariableData::InvalidTime) {
            return aircraft.getEngineConst().interpolate(timestamp, access);
        } else {
            return aircraft.getEngineConst().interpolate(d->skyConnect.getCurrentTimestamp(), access);
        }
    };
}

// PRIVATE SLOTS

void EngineWidget::handleTimestampChanged(qint64 timestamp, TimeVariableData::Access access)
{
    updateUi(timestamp, access);
}
