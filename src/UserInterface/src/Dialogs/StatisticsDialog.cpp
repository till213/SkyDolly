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
#include "../../../Kernel/src/Settings.h"
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

StatisticsDialog::StatisticsDialog(SkyConnectIntf &skyConnect, QWidget *parent) :
    QDialog(parent),
    d(new StatisticsDialogPrivate(skyConnect)),
    ui(new Ui::StatisticsDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    frenchConnection();
}

StatisticsDialog::~StatisticsDialog()
{
    delete ui;
    delete d;
}

// PROTECTED

void StatisticsDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateRecordUi();

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    // Signal sent while recording
    connect(&aircraft, &Aircraft::dataChanged,
            this, &StatisticsDialog::updateRecordUi);
    connect(&Settings::getInstance(), &Settings::recordSampleRateChanged,
            this, &StatisticsDialog::updateRecordUi);

    emit visibilityChanged(true);
}

void StatisticsDialog::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    disconnect(&aircraft, &Aircraft::dataChanged,
               this, &StatisticsDialog::updateRecordUi);

    emit visibilityChanged(false);
}

// PRIVATE

void StatisticsDialog::frenchConnection()
{}

// PRIVATE SLOTS

void StatisticsDialog::updateRecordUi()
{
    const QVector<AircraftData> aircraftData = d->skyConnect.getAircraft().getAllAircraftData();
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
    ui->durationLineEdit->setText(QString::number(lastTimestamp));

    // In KiB
    double size = static_cast<double>(aircraftDataCount * sizeof(AircraftData)) / 1024.0;
    ui->sampleSizeLineEdit->setText(QString::number(size));
}
