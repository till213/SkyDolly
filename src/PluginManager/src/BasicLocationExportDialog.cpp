/**
 * Sky Dolly - The Black Sheep for Your Location Recordings
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
#include <cmath>

#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QDir>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QCheckBox>

#include <Kernel/Settings.h>
#include <Kernel/Unit.h>
#include <Kernel/Enum.h>
#include <Model/Location.h>
#include "Export.h"
#include "LocationExportPluginBaseSettings.h"
#include "BasicLocationExportDialog.h"
#include "ui_BasicLocationExportDialog.h"

struct BasicLocationExportDialogPrivate
{
    BasicLocationExportDialogPrivate(const QString &theFileSuffix, const QString &theFileFilter, LocationExportPluginBaseSettings &thePluginSettings) noexcept
        : fileSuffix(theFileSuffix),
          fileFilter(theFileFilter),
          pluginSettings(thePluginSettings),
          exportButton(nullptr),
          optionWidget(nullptr)
    {}

    QString fileSuffix;
    QString fileFilter;
    LocationExportPluginBaseSettings &pluginSettings;
    QPushButton *exportButton;
    QWidget *optionWidget;
    Unit unit;
};

// PUBLIC

BasicLocationExportDialog::BasicLocationExportDialog(const QString &fileSuffix, const QString &fileFilter, LocationExportPluginBaseSettings &pluginSettings, QWidget *parent) noexcept
    : QDialog(parent),
      ui(std::make_unique<Ui::BasicLocationExportDialog>()),
      d(std::make_unique<BasicLocationExportDialogPrivate>(fileSuffix, fileFilter, pluginSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

BasicLocationExportDialog::~BasicLocationExportDialog() = default;

QString BasicLocationExportDialog::getSelectedFilePath() const noexcept
{
    return QDir::fromNativeSeparators(ui->filePathLineEdit->text());
}

void BasicLocationExportDialog::setSelectedFilePath(const QString &filePath) noexcept
{
     ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
}

void BasicLocationExportDialog::setOptionWidget(QWidget *widget) noexcept
{
    d->optionWidget = widget;
    initOptionUi();
}

// PRIVATE

void BasicLocationExportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->exportButton = ui->defaultButtonBox->addButton(tr("&Export"), QDialogButtonBox::AcceptRole);
    initBasicUi();
    initOptionUi();
}

void BasicLocationExportDialog::initBasicUi() noexcept
{
    ui->filePathLineEdit->setText(QDir::toNativeSeparators(Export::suggestFilePath(d->flight, d->fileSuffix)));
}

void BasicLocationExportDialog::initOptionUi() noexcept
{
    if (d->optionWidget != nullptr) {
        ui->optionGroupBox->setHidden(false);
        std::unique_ptr<QLayout> layout {ui->optionGroupBox->layout()};
        // Any previously existing layout is deleted first, which is what we want
        layout = std::make_unique<QVBoxLayout>();
        layout->addWidget(d->optionWidget);
        // Transfer ownership of the layout back to the optionGroupBox
        ui->optionGroupBox->setLayout(layout.release());
    } else {
        ui->optionGroupBox->setHidden(true);
    }
}

void BasicLocationExportDialog::updateDataGroupBox() noexcept
{
    QString infoText;
    if (isExportUserAircraftOnly()) {
        infoText = tr("The current user aircraft will be exported.");
    } else {
        infoText = tr("%Ln aircraft will be exported.", nullptr, d->flight.count());
    }
    SampleRate::ResamplingPeriod resamplingPeriod = static_cast<SampleRate::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
    std::int64_t samplePoints = estimateNofSamplePoints();
    if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
        infoText.append(" " % tr("The position data will be resampled every %1 milliseconds, resulting in %Ln exported positions.",
                                 nullptr, samplePoints)
                                 .arg(d->unit.formatNumber(Enum::toUnderlyingType(resamplingPeriod), 0)));
    } else {
        infoText.append(" " % tr("The original recorded data will be exported, resulting in total %Ln exported positions.", nullptr, samplePoints));
    }
    ui->infoLabel->setText(infoText);
}

void BasicLocationExportDialog::frenchConnection() noexcept
{
    connect(ui->fileSelectionPushButton, &QPushButton::clicked,
            this, &BasicLocationExportDialog::onFileSelectionButtonClicked);
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &BasicLocationExportDialog::onFilePathChanged);
    connect(ui->openExportCheckBox, &QCheckBox::toggled,
            this, &BasicLocationExportDialog::onDoOpenExportedFilesChanged);
    connect(&d->pluginSettings, &LocationExportPluginBaseSettings::baseSettingsChanged,
            this, &BasicLocationExportDialog::updateUi);
    const QPushButton *resetButton = ui->defaultButtonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, &QPushButton::clicked,
            this, &BasicLocationExportDialog::onRestoreDefaults);
}

// PRIVATE SLOTS

void BasicLocationExportDialog::updateUi() noexcept
{
    const QString filePath = ui->filePathLineEdit->text();
    QFileInfo fileInfo(filePath);
    QFile file(fileInfo.absolutePath());
    d->exportButton->setEnabled(file.exists());
    ui->openExportCheckBox->setChecked(d->pluginSettings.isOpenExportedFilesEnabled());

    updateDataGroupBox();
}

void BasicLocationExportDialog::onFileSelectionButtonClicked() noexcept
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export File"), ui->filePathLineEdit->text(), d->fileFilter);
    if (!filePath.isEmpty()) {
        ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
        d->pluginSettings.setFileDialogSelectedFile(true);
    }
    updateUi();
}

void BasicLocationExportDialog::onFilePathChanged()
{
    d->pluginSettings.setFileDialogSelectedFile(false);
    updateUi();
}

void BasicLocationExportDialog::onDoOpenExportedFilesChanged(bool enable) noexcept
{
    d->pluginSettings.setOpenExportedFilesEnabled(enable);
}

void BasicLocationExportDialog::onRestoreDefaults() noexcept
{
    d->pluginSettings.restoreDefaults();
}
