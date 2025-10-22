/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#include <utility>
#include <cstdint>

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
#include <Model/Attitude.h>
#include <Model/AttitudeData.h>
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
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Connect/Connect.h>
#include "StatisticsDialog.h"
#include "ui_StatisticsDialog.h"

namespace
{
    // Period [ms] over which we count the recorded samples per second
    constexpr std::int64_t SamplesPerSecondPeriod = 10000;
}

struct StatisticsDialogPrivate
{
    Unit unit;
    QShortcut *closeDialogShortcut {nullptr};
    int lastPositionIndex {0};
    int lastAttitudeIndex {0};
};

// PUBLIC

StatisticsDialog::StatisticsDialog(QWidget *parent) noexcept :
    QDialog {parent},
    ui {std::make_unique<Ui::StatisticsDialog>()},
    d {std::make_unique<StatisticsDialogPrivate>()}
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

    // Connection
    auto &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::timestampChanged,
            this, &StatisticsDialog::updateRecordUi);    
    connect(&skyConnectManager, &SkyConnectManager::recordingStarted,
            this, &StatisticsDialog::onRecordingStarted);

    // Flight
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::cleared,
            this, &StatisticsDialog::updateUi);

    emit visibilityChanged(true);
}

void StatisticsDialog::hideEvent(QHideEvent *event) noexcept
{
    QDialog::hideEvent(event);

    // Connection
    auto &skyConnectManager = SkyConnectManager::getInstance();
    disconnect(&skyConnectManager, &SkyConnectManager::timestampChanged,
               this, &StatisticsDialog::updateRecordUi);
    disconnect(&skyConnectManager, &SkyConnectManager::recordingStarted,
               this, &StatisticsDialog::onRecordingStarted);

    // Flight
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::cleared,
               this, &StatisticsDialog::updateUi);

    emit visibilityChanged(false);
}

// PRIVATE

/// \todo DRY: "centrally" define the "S" shortcut (currently also assigned to the corresponding QAction)
void StatisticsDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->closeDialogShortcut = new QShortcut(QKeySequence(tr("S", "Window|Statistics...")), this);
}

void StatisticsDialog::frenchConnection() noexcept
{
    connect(d->closeDialogShortcut, &QShortcut::activated,
            this, &StatisticsDialog::close);
}

std::pair<float, float> StatisticsDialog::calculateRecordedPositionAndAttitudeSamplesPerSecond() const noexcept
{
    float positionSamplesPerSecond {0.0};
    float attitudeSamplesPerSecond {0.0};
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    const auto &aircraft = flight.getUserAircraft();
    const auto &position = aircraft.getPosition();
    if (position.count() > 0) {
        auto lastTimeStamp = position.getLast().timestamp;
        const auto startTimestamp = std::max(lastTimeStamp - ::SamplesPerSecondPeriod, std::int64_t(0));
        int startIndex = d->lastPositionIndex;

        while (position[startIndex].timestamp < startTimestamp) {
            ++startIndex;
        }
        d->lastPositionIndex = startIndex;

        const auto endIndex = position.count() - 1;
        const auto nofSamples = endIndex - startIndex + 1;
        const std::int64_t period = position[endIndex].timestamp - position[startIndex].timestamp;
        if (period > 0) {
            positionSamplesPerSecond = static_cast<float>(nofSamples) * 1000.0f / (static_cast<float>(period));
        }
    }

    const auto &attitude = aircraft.getAttitude();
    if (attitude.count() > 0) {
        auto lastTimeStamp = attitude.getLast().timestamp;
        const auto startTimestamp = std::max(lastTimeStamp - ::SamplesPerSecondPeriod, std::int64_t(0));
        int startIndex = d->lastAttitudeIndex;

        while (attitude[startIndex].timestamp < startTimestamp) {
            ++startIndex;
        }
        d->lastAttitudeIndex = startIndex;

        const auto endIndex = attitude.count() - 1;
        const auto nofSamples = endIndex - startIndex + 1;
        const std::int64_t period = attitude[endIndex].timestamp - attitude[startIndex].timestamp;
        if (period > 0) {
            attitudeSamplesPerSecond = static_cast<float>(nofSamples) * 1000.0f / (static_cast<float>(period));
        }
    }

    return std::make_pair(positionSamplesPerSecond, attitudeSamplesPerSecond);
}

// PRIVATE SLOTS

void StatisticsDialog::updateUi() noexcept
{
    updateRecordUi(SkyConnectManager::getInstance().getCurrentTimestamp());
}

void StatisticsDialog::updateRecordUi(std::int64_t timestamp) noexcept
{
    const auto &flight = Logbook::getInstance().getCurrentFlight();

    // Samples per second
    const auto &skyConnectManager = SkyConnectManager::getInstance();
    if (skyConnectManager.getState() == Connect::State::Recording) {
        auto positionAndAttitudeSamplesPerSecond = calculateRecordedPositionAndAttitudeSamplesPerSecond();
        ui->positionSamplesPerSecondLineEdit->setText(d->unit.formatHz(positionAndAttitudeSamplesPerSecond.first));
        ui->attitudeSamplesPerSecondLineEdit->setText(d->unit.formatHz(positionAndAttitudeSamplesPerSecond.second));
        ui->durationLineEdit->setText(d->unit.formatElapsedTime(timestamp));
    } else {
        ui->positionSamplesPerSecondLineEdit->clear();
        ui->attitudeSamplesPerSecondLineEdit->clear();
        ui->durationLineEdit->setText(d->unit.formatElapsedTime(flight.getTotalDurationMSec()));
    }

    std::size_t totalCount = 0;
    std::size_t totalSize = 0;
    for (const auto &aircraft : flight) {
        const auto positionDataCount = aircraft.getPosition().count();
        const auto attitudeDataCount = aircraft.getAttitude().count();
        const auto engineDataCount = aircraft.getEngine().count();
        const auto primaryFlightControlDataCount = aircraft.getPrimaryFlightControl().count();
        const auto secondaryFlightControlDataCount = aircraft.getSecondaryFlightControl().count();
        const auto aircraftHandleDataCount = aircraft.getAircraftHandle().count();
        const auto lightDataCount = aircraft.getLight().count();
        totalCount = totalCount + attitudeDataCount + positionDataCount + engineDataCount + primaryFlightControlDataCount + secondaryFlightControlDataCount + aircraftHandleDataCount + lightDataCount;

        const auto positionDataSize = positionDataCount * sizeof(PositionData);
        const auto attitudeDataSize = attitudeDataCount * sizeof(AttitudeData);
        const auto engineDataSize = engineDataCount * sizeof(EngineData);
        const auto primaryFlightControlDataSize = primaryFlightControlDataCount * sizeof(PrimaryFlightControlData);
        const auto secondaryFlightControlDataSize = secondaryFlightControlDataCount * sizeof(SecondaryFlightControlData);
        const auto aircraftHandleDataSize = aircraftHandleDataCount * sizeof(AircraftHandleData);
        const auto lightDataSize = lightDataCount * sizeof(LightData);
        totalSize = totalSize + positionDataSize + attitudeDataSize + engineDataSize + primaryFlightControlDataSize + secondaryFlightControlDataSize + aircraftHandleDataSize + lightDataSize;
    }

    ui->sampleCountLineEdit->setText(QString::number(totalCount));
    ui->sampleSizeLineEdit->setText(d->unit.formatMemory(totalSize));
}

void StatisticsDialog::onRecordingStarted() noexcept
{
    d->lastPositionIndex = 0;
    d->lastAttitudeIndex = 0;
}
