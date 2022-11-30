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
#include <optional>

#include <QWidget>
#include <QDialog>
#include <QShortcut>
#include <QShowEvent>
#include <QHideEvent>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Settings.h>
#include <Kernel/Unit.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/PrimaryFlightControl.h>
#include <Model/PrimaryFlightControlData.h>
#include <Model/SecondaryFlightControl.h>
#include <Model/SecondaryFlightControlData.h>
#include <Model/AircraftHandle.h>
#include <Model/AircraftHandleData.h>
#include <Model/Light.h>
#include <Model/LightData.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/Connect.h>
#include "StatisticsDialog.h"
#include "ui_StatisticsDialog.h"

struct StatisticsDialogPrivate
{
    Unit unit;
    QShortcut *closeDialogShortcut {nullptr};
};

// PUBLIC

StatisticsDialog::StatisticsDialog(QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<StatisticsDialogPrivate>()),
    ui(std::make_unique<Ui::StatisticsDialog>())
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

StatisticsDialog::~StatisticsDialog() = default;

// PROTECTED

void StatisticsDialog::showEvent(QShowEvent *event) noexcept
{
    QDialog::showEvent(event);
    updateUi();

    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::timestampChanged,
            this, &StatisticsDialog::updateRecordUi);
    connect(&Settings::getInstance(), &Settings::recordingSampleRateChanged,
            this, &StatisticsDialog::updateRecordingSampleRate);

    emit visibilityChanged(true);
}

void StatisticsDialog::hideEvent(QHideEvent *event) noexcept
{
    QDialog::hideEvent(event);
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    disconnect(&skyConnectManager, &SkyConnectManager::timestampChanged,
               this, &StatisticsDialog::updateRecordUi);
    disconnect(&Settings::getInstance(), &Settings::recordingSampleRateChanged,
               this, &StatisticsDialog::updateRecordingSampleRate);

    emit visibilityChanged(false);
}

// PRIVATE

/// \todo DRY: "centrally" define the "S" shortcut (currently also assigned to the corresponding QAction)
void StatisticsDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->closeDialogShortcut = new QShortcut(QKeySequence(tr("S", "Window|Statistics...")), this);
}

void StatisticsDialog::updateUi() noexcept
{
    updateRecordingSampleRate();
    updateRecordUi(SkyConnectManager::getInstance().getCurrentTimestamp());
}

void StatisticsDialog::frenchConnection() noexcept
{
    connect(d->closeDialogShortcut, &QShortcut::activated,
            this, &StatisticsDialog::close);
}

// PRIVATE SLOTS

void StatisticsDialog::updateRecordingSampleRate() noexcept
{
    if (Settings::getInstance().getRecordingSampleRate() != SampleRate::SampleRate::Auto) {
        ui->recordingSampleRateLineEdit->setText(d->unit.formatHz(Settings::getInstance().getRecordingSampleRateValue()));
    } else {
        ui->recordingSampleRateLineEdit->setText(tr("Auto"));
    }
}

void StatisticsDialog::updateRecordUi(std::int64_t timestamp) noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();

    // Samples per second
    const SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    if (skyConnectManager.getState() == Connect::State::Recording) {
        const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = skyConnectManager.getCurrentSkyConnect();
        if (skyConnect) {
            ui->samplesPerSecondLineEdit->setText(d->unit.formatHz(skyConnect->get().calculateRecordedSamplesPerSecond()));
            ui->durationLineEdit->setText(d->unit.formatElapsedTime(timestamp));
        } else {
            ui->samplesPerSecondLineEdit->clear();
        }
    } else {
        ui->samplesPerSecondLineEdit->clear();
        ui->durationLineEdit->setText(d->unit.formatElapsedTime(flight.getTotalDurationMSec()));
    }

    std::size_t totalCount = 0;
    std::size_t totalSize = 0;
    for (const auto &aircraft : flight) {
        const std::size_t positionDataCount = aircraft.getPosition().count();
        const std::size_t engineDataCount = aircraft.getEngine().count();
        const std::size_t primaryFlightControlDataCount = aircraft.getPrimaryFlightControl().count();
        const std::size_t secondaryFlightControlDataCount = aircraft.getSecondaryFlightControl().count();
        const std::size_t aircraftHandleDataCount = aircraft.getAircraftHandle().count();
        const std::size_t lightDataCount = aircraft.getLight().count();
        totalCount = totalCount + positionDataCount + engineDataCount + primaryFlightControlDataCount + secondaryFlightControlDataCount + aircraftHandleDataCount + lightDataCount;

        const std::size_t positionDataSize = positionDataCount * sizeof(PositionData);
        const std::size_t engineDataSize = engineDataCount * sizeof(EngineData);
        const std::size_t primaryFlightControlDataSize = primaryFlightControlDataCount * sizeof(PrimaryFlightControlData);
        const std::size_t secondaryFlightControlDataSize = secondaryFlightControlDataCount * sizeof(SecondaryFlightControlData);
        const std::size_t aircraftHandleDataSize = aircraftHandleDataCount * sizeof(AircraftHandleData);
        const std::size_t lightDataSize = lightDataCount * sizeof(LightData);
        totalSize = totalSize + positionDataSize + engineDataSize + primaryFlightControlDataSize + secondaryFlightControlDataSize + aircraftHandleDataSize + lightDataSize;
    }

    ui->sampleCountLineEdit->setText(QString::number(totalCount));
    ui->sampleSizeLineEdit->setText(d->unit.formatMemory(totalSize));
}
