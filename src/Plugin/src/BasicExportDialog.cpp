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
#include <QColorDialog>

#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/SampleRate.h"
#include "../../Kernel/src/Unit.h"
#include "../../Kernel/src/Enum.h"
#include "../../Model/src/Logbook.h"
#include "../../Model/src/Flight.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/Position.h"
#include "../../Model/src/SimType.h"
#include "BasicExportDialog.h"
#include "ui_BasicExportDialog.h"

class BasicExportDialogPrivate
{
public:
    BasicExportDialogPrivate(const QString &theFileFilter) noexcept
        : fileFilter(theFileFilter),
          exportButton(nullptr),
          optionWidget(nullptr)
    {}

    QString fileFilter;
    SampleRate::ResamplingPeriod resamplingPeriod;
    QPushButton *exportButton;
    QWidget *optionWidget;
    Unit unit;
};

// PUBLIC

BasicExportDialog::BasicExportDialog(const QString &fileExtension, QWidget *parent) noexcept
    : QDialog(parent),
      ui(new Ui::BasicExportDialog),
      d(std::make_unique<BasicExportDialogPrivate>(fileExtension))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("BasicExportDialog::BasicExportDialog: CREATED");
#endif
}

BasicExportDialog::~BasicExportDialog() noexcept
{
    delete ui;
#ifdef DEBUG
    qDebug("BasicExportDialog::~BasicExportDialog: DELETED");
#endif
}

QString BasicExportDialog::getSelectedFilePath() const noexcept
{
    return QDir::fromNativeSeparators(ui->filePathLineEdit->text());
}

void BasicExportDialog::setSelectedFilePath(const QString &filePath) noexcept
{
     ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
}

bool BasicExportDialog::doOpenExportedFile() const noexcept
{
    return ui->openExportCheckBox->isChecked();
}

SampleRate::ResamplingPeriod BasicExportDialog::getResamplingPeriod() const noexcept
{
    return d->resamplingPeriod;
}

void BasicExportDialog::setResamplingPeriod(SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    d->resamplingPeriod = resamplingPeriod;
    updateUi();
}

void BasicExportDialog::setOptionWidget(QWidget *widget) noexcept
{
    d->optionWidget = widget;
    initOptionUi();
}

// PRIVATE

void BasicExportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->exportButton = ui->defaultButtonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);
    initBasicUi();
    initOptionUi();
}

void BasicExportDialog::initBasicUi() noexcept
{
    // Resampling
    ui->resamplingComboBox->addItem(QString("1/10 Hz") % " (" % tr("less data, less accuracy") % ")", Enum::toUnderlyingType(SampleRate::ResamplingPeriod::ATenthHz));
    ui->resamplingComboBox->addItem("1/5 Hz", Enum::toUnderlyingType(SampleRate::ResamplingPeriod::AFifthHz));
    ui->resamplingComboBox->addItem(QString("1 Hz") % " (" % tr("good accuracy") % ")", Enum::toUnderlyingType(SampleRate::ResamplingPeriod::OneHz));
    ui->resamplingComboBox->addItem("2 Hz", Enum::toUnderlyingType(SampleRate::ResamplingPeriod::TwoHz));
    ui->resamplingComboBox->addItem("5 Hz", Enum::toUnderlyingType(SampleRate::ResamplingPeriod::FiveHz));
    ui->resamplingComboBox->addItem("10 Hz", Enum::toUnderlyingType(SampleRate::ResamplingPeriod::TenHz));
    ui->resamplingComboBox->addItem(tr("Original data"), Enum::toUnderlyingType(SampleRate::ResamplingPeriod::Original));
}

void BasicExportDialog::initOptionUi() noexcept
{
    if (d->optionWidget != nullptr) {
        ui->optionGroupBox->setHidden(false);
        QLayout *layout = ui->optionGroupBox->layout();
        if (layout == nullptr) {
            delete layout;
        }
        layout = new QVBoxLayout();
        ui->optionGroupBox->setLayout(layout);
        layout->addWidget(d->optionWidget);
    } else {
        ui->optionGroupBox->setHidden(true);
    }
}

void BasicExportDialog::updateInfoUi() noexcept
{
    QString infoText;
    SampleRate::ResamplingPeriod resamplingPeriod = static_cast<SampleRate::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
    std::int64_t samplePoints = estimateNofSamplePoints();
    if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
        infoText = tr("The position data is resampled every %1 milliseconds, resulting in approximately %2 exported positions in total.")
                      .arg(d->unit.formatNumber(Enum::toUnderlyingType(resamplingPeriod), 0), d->unit.formatNumber(samplePoints, 0));
    } else {
        infoText = tr("WARNING: exporting the original position data may result in large files. The file viewer performance may "
                      "drastically slow down, or the exported data may not even be displayed at all.\n\nIn total %1 positions will be exported.")
                      .arg(d->unit.formatNumber(samplePoints, 0));
    }
    ui->infoLabel->setText(infoText);
}

void BasicExportDialog::frenchConnection() noexcept
{
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &BasicExportDialog::updateUi);

    QPushButton *resetButton = ui->defaultButtonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, &QPushButton::clicked,
            this, &BasicExportDialog::onRestoreDefaults);
}

std::int64_t BasicExportDialog::estimateNofSamplePoints() noexcept
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

void BasicExportDialog::updateUi() noexcept
{
    const QString filePath = ui->filePathLineEdit->text();
    QFileInfo fileInfo(filePath);
    QFile file(fileInfo.absolutePath());
    d->exportButton->setEnabled(file.exists());

    int currentIndex = 0;
    while (currentIndex < ui->resamplingComboBox->count() &&
           static_cast<SampleRate::ResamplingPeriod>(ui->resamplingComboBox->itemData(currentIndex).toInt()) != d->resamplingPeriod) {
        ++currentIndex;
    }
    ui->resamplingComboBox->setCurrentIndex(currentIndex);

    updateInfoUi();
}

void BasicExportDialog::onRestoreDefaults() noexcept
{
    d->resamplingPeriod = SampleRate::DefaultResamplingPeriod;
    updateUi();
    emit restoreDefaultOptions();
}

void BasicExportDialog::onFileSelectionChanged() noexcept
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export file..."), ui->filePathLineEdit->text(), d->fileFilter);
    if (!filePath.isEmpty()) {
        ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
    }
    updateUi();
}

void BasicExportDialog::onResamplingOptionChanged([[maybe_unused]] int index) noexcept
{
    d->resamplingPeriod = static_cast<SampleRate::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
    updateInfoUi();
}
