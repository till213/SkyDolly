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

#include "../../../Kernel/src/Settings.h"
#include "../../../Kernel/src/Unit.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/Position.h"
#include "../../../Model/src/PositionData.h"
#include "../../../Model/src/Engine.h"
#include "../../../Model/src/EngineData.h"
#include "../../../Model/src/PrimaryFlightControl.h"
#include "../../../Model/src/PrimaryFlightControlData.h"
#include "../../../Model/src/SecondaryFlightControl.h"
#include "../../../Model/src/SecondaryFlightControlData.h"
#include "../../../Model/src/AircraftHandle.h"
#include "../../../Model/src/AircraftHandleData.h"
#include "../../../Model/src/Light.h"
#include "../../../Model/src/LightData.h"
#include "../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "StatisticsDialog.h"
#include "ui_StatisticsDialog.h"

class StatisticsDialogPrivate
{
public:
    StatisticsDialogPrivate() noexcept
    {}

    Unit unit;
};

// PUBLIC

StatisticsDialog::StatisticsDialog(QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<StatisticsDialogPrivate>()),
    ui(std::make_unique<Ui::StatisticsDialog>())
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
    frenchConnection();
}

StatisticsDialog::~StatisticsDialog() noexcept
{}

// PROTECTED

void StatisticsDialog::showEvent(QShowEvent *event) noexcept
{
    QDialog::showEvent(event);
    updateRecordUi();

    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    // Signal sent while recording
    connect(&aircraft, &Aircraft::dataChanged,
            this, &StatisticsDialog::updateRecordUi);
    connect(&Settings::getInstance(), &Settings::recordingSampleRateChanged,
            this, &StatisticsDialog::updateRecordUi);

    emit visibilityChanged(true);
}

void StatisticsDialog::hideEvent(QHideEvent *event) noexcept
{
    QDialog::hideEvent(event);
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    disconnect(&aircraft, &Aircraft::dataChanged,
               this, &StatisticsDialog::updateRecordUi);

    emit visibilityChanged(false);
}

// PRIVATE

void StatisticsDialog::frenchConnection() noexcept
{}

// PRIVATE SLOTS

void StatisticsDialog::updateRecordUi() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    if (Settings::getInstance().getRecordingSampleRate() != SampleRate::SampleRate::Auto) {
        ui->recordingSampleRateLineEdit->setText(d->unit.formatHz(Settings::getInstance().getRecordingSampleRateValue()));
    } else {
        ui->recordingSampleRateLineEdit->setText(tr("Auto"));
    }

    // Samples per second
    const auto skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().getState() == Connect::State::Recording) {
            ui->samplesPerSecondLineEdit->setText(d->unit.formatHz(skyConnect->get().calculateRecordedSamplesPerSecond()));
        } else {
            ui->samplesPerSecondLineEdit->clear();
        }
    } else {
        ui->samplesPerSecondLineEdit->clear();
    }

    qint64 totalCount = 0;
    qint64 totalSize = 0;
    for (const auto &aircraft : flight) {
        const int positionDataCount = aircraft->getPositionConst().count();
        const int engineDataCount = aircraft->getEngineConst().count();
        const int primaryFlightControlDataCount = aircraft->getPrimaryFlightControlConst().count();
        const int secondaryFlightControlDataCount = aircraft->getSecondaryFlightControlConst().count();
        const int aircraftHandleDataCount = aircraft->getAircraftHandleConst().count();
        const int lightDataCount = aircraft->getLightConst().count();
        totalCount = totalCount + positionDataCount + engineDataCount + primaryFlightControlDataCount + secondaryFlightControlDataCount + aircraftHandleDataCount + lightDataCount;

        const qint64 positionDataSize = positionDataCount * sizeof(PositionData);
        const qint64 engineDataSize = engineDataCount * sizeof(EngineData);
        const qint64 primaryFlightControlDataSize = primaryFlightControlDataCount * sizeof(PrimaryFlightControlData);
        const qint64 secondaryFlightControlDataSize = secondaryFlightControlDataCount * sizeof(SecondaryFlightControlData);
        const qint64 aircraftHandleDataSize = aircraftHandleDataCount * sizeof(AircraftHandleData);
        const qint64 lightDataSize = lightDataCount * sizeof(LightData);
        totalSize = totalSize + positionDataSize + engineDataSize + primaryFlightControlDataSize + secondaryFlightControlDataSize + aircraftHandleDataSize + lightDataSize;
    }

    ui->sampleCountLineEdit->setText(QString::number(totalCount));
    ui->durationLineEdit->setText(d->unit.formatElapsedTime(flight.getTotalDurationMSec()));
    ui->sampleSizeLineEdit->setText(d->unit.formatMemory(totalSize));
}
