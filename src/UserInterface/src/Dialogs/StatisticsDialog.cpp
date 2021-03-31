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
#include "../../../Model/src/World.h"
#include "../../../Model/src/Scenario.h"
#include "../../../Model/src/Aircraft.h"
#include "../../../Model/src/AircraftData.h"
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
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "StatisticsDialog.h"
#include "ui_StatisticsDialog.h"

class StatisticsDialogPrivate
{
public:
    StatisticsDialogPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
};

// PUBLIC

StatisticsDialog::StatisticsDialog(SkyConnectIntf &skyConnect, QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<StatisticsDialogPrivate>(skyConnect)),
    ui(std::make_unique<Ui::StatisticsDialog>())
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    frenchConnection();
}

StatisticsDialog::~StatisticsDialog() noexcept
{
}

// PROTECTED

void StatisticsDialog::showEvent(QShowEvent *event) noexcept
{
    Q_UNUSED(event)

    updateRecordUi();

    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();
    // Signal sent while recording
    connect(&aircraft, &Aircraft::dataChanged,
            this, &StatisticsDialog::updateRecordUi);
    connect(&Settings::getInstance(), &Settings::recordSampleRateChanged,
            this, &StatisticsDialog::updateRecordUi);

    emit visibilityChanged(true);
}

void StatisticsDialog::hideEvent(QHideEvent *event) noexcept
{
    Q_UNUSED(event)

    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();
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
    const Scenario &scenario = World::getInstance().getCurrentScenario();
    const Aircraft &aircraft = scenario.getUserAircraft();

    if (Settings::getInstance().getRecordSampleRate() != SampleRate::SampleRate::Auto) {
        ui->recordSampleRateLineEdit->setText(QString::number(Settings::getInstance().getRecordSampleRateValue()));
    } else {
        ui->recordSampleRateLineEdit->setText(tr("Auto"));
    }

    // Samples per second
    if (d->skyConnect.getState() == Connect::State::Recording) {
        ui->samplesPerSecondLineEdit->setText(QString::number(d->skyConnect.calculateRecordedSamplesPerSecond(), 'f', 1));
    } else {
        ui->samplesPerSecondLineEdit->clear();
    }
    const QVector<AircraftData> &aircraftData = aircraft.getAll();
    const QVector<EngineData> &engineData = aircraft.getEngine().getAll();
    const QVector<PrimaryFlightControlData> &primaryFlightControlData = aircraft.getPrimaryFlightControl().getAll();
    const QVector<SecondaryFlightControlData> &secondaryFlightControlData = aircraft.getSecondaryFlightControl().getAll();
    const QVector<AircraftHandleData> &aircraftHandleData = aircraft.getAircraftHandle().getAll();
    const QVector<LightData> &lightData = aircraft.getLight().getAll();
    const int totalCount = aircraftData.count() + engineData.count() + primaryFlightControlData.count() + secondaryFlightControlData.count() + aircraftHandleData.count() + lightData.count();
    ui->sampleCountLineEdit->setText(QString::number(totalCount));

    ui->durationLineEdit->setText(QString::number(scenario.getTotalDuration() / 1000.0, 'f', 1));

    // In KiB
    const double aircraftDataSize = static_cast<double>(aircraftData.count()  * sizeof(AircraftData)) / 1024.0;
    const double engineDataSize = static_cast<double>(engineData.count()  * sizeof(EngineData)) / 1024.0;
    const double primaryFlightControlDataSize = static_cast<double>(primaryFlightControlData.count()  * sizeof(PrimaryFlightControlData)) / 1024.0;
    const double secondaryFlightControlDataSize = static_cast<double>(secondaryFlightControlData.count()  * sizeof(SecondaryFlightControlData)) / 1024.0;
    const double aircraftHandleDataSize = static_cast<double>(aircraftHandleData.count()  * sizeof(AircraftHandleData)) / 1024.0;
    const double lightDataSize = static_cast<double>(lightData.count()  * sizeof(LightData)) / 1024.0;

    const double totalSize = aircraftDataSize + engineDataSize + primaryFlightControlDataSize + secondaryFlightControlDataSize + aircraftHandleDataSize + lightDataSize;
    ui->sampleSizeLineEdit->setText(QString::number(totalSize, 'f', 1));
}
