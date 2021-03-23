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
    const Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();
    const QVector<AircraftData> aircraftData = aircraft.getAllAircraftData();
    if (Settings::getInstance().getRecordSampleRate() != SampleRate::SampleRate::Auto) {
        ui->recordSampleRateLineEdit->setText(QString::number(Settings::getInstance().getRecordSampleRateValue()));
    } else {
        ui->recordSampleRateLineEdit->setText(tr("Auto"));
    }

    // Samples per Second
    if (d->skyConnect.getState() == Connect::State::Recording) {
        ui->samplesPerSecondLineEdit->setText(QString::number(d->skyConnect.calculateRecordedSamplesPerSecond()));
    } else {
        ui->samplesPerSecondLineEdit->clear();
    }
    int aircraftDataCount = aircraftData.count();
    ui->sampleCountLineEdit->setText(QString::number(aircraftDataCount));

    qint64 lastTimestamp;
    if (aircraftData.count() > 0) {
        lastTimestamp = aircraftData.last().timestamp;
    } else {
        lastTimestamp = 0;
    }
    ui->durationLineEdit->setText(QString::number(lastTimestamp / 1000.0));

    // In KiB
    double size = static_cast<double>(aircraftDataCount * sizeof(AircraftData)) / 1024.0;
    ui->sampleSizeLineEdit->setText(QString::number(size));
}
