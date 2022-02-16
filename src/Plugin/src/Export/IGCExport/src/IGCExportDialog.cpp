/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <utility>
#include <limits>
#include <cstdint>

#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QDir>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QButtonGroup>

#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../../src/Export.h"
#include "IGCExportDialog.h"
#include "IGCExportSettings.h"
#include "ui_IGCExportDialog.h"

class IGCExportDialogPrivate
{
public:
    IGCExportDialogPrivate(IGCExportSettings &theExportSettings) noexcept
        : exportSettings(theExportSettings),
          exportButton(nullptr)
    {}

    IGCExportSettings &exportSettings;
    QPushButton *exportButton;
    Unit unit;
};

// PUBLIC

IGCExportDialog::IGCExportDialog(IGCExportSettings &exportSettings, QWidget *parent) noexcept
    : QDialog(parent),
      ui(new Ui::IGCExportDialog),
      d(std::make_unique<IGCExportDialogPrivate>(exportSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

IGCExportDialog::~IGCExportDialog() noexcept
{
    delete ui;
#ifdef DEBUG
    qDebug("IGCExportDialog::~IGCExportDialog: DELETED");
#endif
}

QString IGCExportDialog::getSelectedFilePath() const noexcept
{
    return ui->filePathLineEdit->text();
}

bool IGCExportDialog::doOpenExportedFile() const noexcept
{
    return ui->openExportCheckBox->isChecked();
}

// PRIVATE

void IGCExportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->exportButton = ui->buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);

    // File path
    ui->filePathLineEdit->setText(QDir::toNativeSeparators(Export::suggestFilePath(FileExtension)));

    // Resampling
    ui->resamplingComboBox->addItem(QString("1/10 Hz") % " (" % tr("less data, less accuracy") % ")", Enum::toUnderlyingType(IGCExportSettings::ResamplingPeriod::ATenthHz));
    ui->resamplingComboBox->addItem("1/5 Hz", Enum::toUnderlyingType(IGCExportSettings::ResamplingPeriod::AFifthHz));
    ui->resamplingComboBox->addItem(QString("1 Hz") % " (" % tr("good accuracy") % ")", Enum::toUnderlyingType(IGCExportSettings::ResamplingPeriod::OneHz));
    ui->resamplingComboBox->addItem("2 Hz", Enum::toUnderlyingType(IGCExportSettings::ResamplingPeriod::TwoHz));
    ui->resamplingComboBox->addItem("5 Hz", Enum::toUnderlyingType(IGCExportSettings::ResamplingPeriod::FiveHz));
    ui->resamplingComboBox->addItem("10 Hz", Enum::toUnderlyingType(IGCExportSettings::ResamplingPeriod::TenHz));
    ui->resamplingComboBox->addItem(tr("Original data"), Enum::toUnderlyingType(IGCExportSettings::ResamplingPeriod::Original));
}

void IGCExportDialog::updateInfoUi() noexcept
{
    QString infoText;
    IGCExportSettings::ResamplingPeriod resamplingPeriod = static_cast<IGCExportSettings::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
    std::int64_t samplePoints = estimateNofSamplePoints();
    if (resamplingPeriod != IGCExportSettings::ResamplingPeriod::Original) {
        infoText = tr("The position data is resampled every %1 milliseconds, resulting in approximately %2 exported positions in total.")
                      .arg(d->unit.formatNumber(Enum::toUnderlyingType(resamplingPeriod), 0), d->unit.formatNumber(samplePoints, 0));
    } else {
        infoText = tr("WARNING: depending on the original sampling frequencey exporting the original position data may result in large IGC files. The IGC viewer performance may "
                      "drastically slow down.\n\nIn total %1 positions will be exported.")
                      .arg(d->unit.formatNumber(samplePoints, 0));
    }
    ui->infoLabel->setText(infoText);
}

void IGCExportDialog::updateFlightUi() noexcept
{
    ui->pilotNameLineEdit->setText(d->exportSettings.pilotName);
    ui->coPilotNameLineEdit->setText(d->exportSettings.coPilotName);
}

void IGCExportDialog::frenchConnection() noexcept
{
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &IGCExportDialog::updateUi);

    QPushButton *resetButton = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, &QPushButton::clicked,
            this, &IGCExportDialog::restoreDefaults);
}

std::int64_t IGCExportDialog::estimateNofSamplePoints() noexcept
{
    Flight &flight = Logbook::getInstance().getInstance().getCurrentFlight();
    std::int64_t nofSamplePoints = 0;
    const std::int64_t period = ui->resamplingComboBox->currentData().toInt();
    if (period != 0) {
        for (const auto &aircraft : flight) {
            std::int64_t duration = aircraft->getDurationMSec();
            nofSamplePoints += qRound(static_cast<double>(duration) / static_cast<double>(period));
        }
    } else {
        // Count the actual position sample points
        for (const auto &aircraft : flight) {
            nofSamplePoints += aircraft->getPositionConst().count();
        }
    }
    return nofSamplePoints;
}

// PRIVATE SLOTS

void IGCExportDialog::updateUi() noexcept
{
    const QString filePath = ui->filePathLineEdit->text();
    QFileInfo fileInfo(filePath);
    QFile file(fileInfo.absolutePath());
    d->exportButton->setEnabled(file.exists());

    int currentIndex = 0;
    while (currentIndex < ui->resamplingComboBox->count() &&
           static_cast<IGCExportSettings::ResamplingPeriod>(ui->resamplingComboBox->itemData(currentIndex).toInt()) != d->exportSettings.resamplingPeriod) {
        ++currentIndex;
    }
    ui->resamplingComboBox->setCurrentIndex(currentIndex);

    updateInfoUi();
    updateFlightUi();
}

void IGCExportDialog::restoreDefaults() noexcept
{
    d->exportSettings.restoreDefaults();
    updateUi();
}

void IGCExportDialog::on_fileSelectionPushButton_clicked() noexcept
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export IGC"), ui->filePathLineEdit->text(), QString("*.igc"));
    if (!filePath.isEmpty()) {
        ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
    }
    updateUi();
}

void IGCExportDialog::on_resamplingComboBox_activated([[maybe_unused]] int index) noexcept
{
    d->exportSettings.resamplingPeriod = static_cast<IGCExportSettings::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
    updateInfoUi();
}

void IGCExportDialog::on_pilotNameLineEdit_editingFinished() noexcept
{
    d->exportSettings.pilotName = ui->pilotNameLineEdit->text();
    updateFlightUi();
}

void IGCExportDialog::on_coPilotNameLineEdit_editingFinished() noexcept
{
    d->exportSettings.coPilotName = ui->coPilotNameLineEdit->text();
    updateFlightUi();
}
