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
#include <Kernel/SampleRate.h>
#include <Kernel/Unit.h>
#include <Kernel/Enum.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/SimType.h>
#include <Flight/FlightExportPluginBaseSettings.h>
#include <Flight/BasicFlightExportDialog.h>
#include "Export.h"
#include "ui_BasicFlightExportDialog.h"

struct BasicFlightExportDialogPrivate
{
    BasicFlightExportDialogPrivate(const Flight &theFlight, QString fileExtension, QString fileFilter, FlightExportPluginBaseSettings &thePluginSettings) noexcept
        : flight(theFlight),
          fileExtension(std::move(fileExtension)),
          fileFilter(std::move(fileFilter)),
          pluginSettings(thePluginSettings)
    {}

    const Flight &flight;
    QString fileExtension;
    QString fileFilter;
    FlightExportPluginBaseSettings &pluginSettings;
    QPushButton *exportButton {nullptr};
    QWidget *optionWidget {nullptr};
    Unit unit;
    bool fileDialogSelectedFile {false};
};

// PUBLIC

BasicFlightExportDialog::BasicFlightExportDialog(const Flight &flight, const QString &fileExtension, const QString &fileFilter, FlightExportPluginBaseSettings &pluginSettings, QWidget *parent) noexcept
    : QDialog(parent),
      ui(std::make_unique<Ui::BasicFlightExportDialog>()),
      d(std::make_unique<BasicFlightExportDialogPrivate>(flight, fileExtension, fileFilter, pluginSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

BasicFlightExportDialog::~BasicFlightExportDialog() = default;

QString BasicFlightExportDialog::getSelectedFilePath() const noexcept
{
    return QDir::fromNativeSeparators(ui->filePathLineEdit->text());
}

void BasicFlightExportDialog::setSelectedFilePath(const QString &filePath) noexcept
{
     ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
}

void BasicFlightExportDialog::setOptionWidget(QWidget *widget) noexcept
{
    d->optionWidget = widget;
    initOptionUi();
}

bool BasicFlightExportDialog::isFileDialogSelectedFile() const noexcept
{
    return d->fileDialogSelectedFile;
}

// PRIVATE

void BasicFlightExportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->exportButton = ui->defaultButtonBox->addButton(tr("&Export"), QDialogButtonBox::AcceptRole);
    initBasicUi();
    initOptionUi();
}

void BasicFlightExportDialog::initBasicUi() noexcept
{
    ui->filePathLineEdit->setText(QDir::toNativeSeparators(Export::suggestFlightFilePath(d->flight, d->fileExtension)));

    // Formation export
    if (d->pluginSettings.isFormationExportSupported(FlightExportPluginBaseSettings::FormationExport::UserAircraftOnly)) {
        ui->formationExportComboBox->addItem(tr("User aircraft only"), Enum::underly(FlightExportPluginBaseSettings::FormationExport::UserAircraftOnly));
    }
    if (d->pluginSettings.isFormationExportSupported(FlightExportPluginBaseSettings::FormationExport::AllAircraftOneFile)) {
        ui->formationExportComboBox->addItem(tr("All aircraft (single file)"), Enum::underly(FlightExportPluginBaseSettings::FormationExport::AllAircraftOneFile));
    }
    if (d->pluginSettings.isFormationExportSupported(FlightExportPluginBaseSettings::FormationExport::AllAircraftSeparateFiles)) {
        ui->formationExportComboBox->addItem(tr("All aircraft (separate files)"), Enum::underly(FlightExportPluginBaseSettings::FormationExport::AllAircraftSeparateFiles));
    }

    // Resampling
    if (d->pluginSettings.isResamplingSupported()) {
        ui->resamplingComboBox->addItem(QString("1/10 Hz") % " (" % tr("smaller file size, less accuracy") % ")", Enum::underly(SampleRate::ResamplingPeriod::ATenthHz));
        ui->resamplingComboBox->addItem("1/5 Hz", Enum::underly(SampleRate::ResamplingPeriod::AFifthHz));
        ui->resamplingComboBox->addItem(QString("1 Hz") % " (" % tr("good accuracy") % ")", Enum::underly(SampleRate::ResamplingPeriod::OneHz));
        ui->resamplingComboBox->addItem("2 Hz", Enum::underly(SampleRate::ResamplingPeriod::TwoHz));
        ui->resamplingComboBox->addItem("5 Hz", Enum::underly(SampleRate::ResamplingPeriod::FiveHz));
        ui->resamplingComboBox->addItem("10 Hz (larger file size, greater accuracy", Enum::underly(SampleRate::ResamplingPeriod::TenHz));
    }
    ui->resamplingComboBox->addItem(tr("Original data (no resampling)"), Enum::underly(SampleRate::ResamplingPeriod::Original));
}

void BasicFlightExportDialog::initOptionUi() noexcept
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

void BasicFlightExportDialog::updateDataGroupBox() noexcept
{
    QString infoText;
    if (isExportUserAircraftOnly()) {
        infoText = tr("The current user aircraft will be exported.");
    } else {
        infoText = tr("%Ln aircraft will be exported.", nullptr, static_cast<int>(d->flight.count()));
    }
    SampleRate::ResamplingPeriod resamplingPeriod = static_cast<SampleRate::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt());
    std::size_t samplePoints = estimateNofSamplePoints();
    if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
        infoText.append(" " % tr("The position data will be resampled every %1 milliseconds, resulting in %Ln exported positions.",
                                 nullptr, samplePoints)
                                 .arg(d->unit.formatNumber(Enum::underly(resamplingPeriod), 0)));
    } else {
        infoText.append(" " % tr("The original recorded data will be exported, resulting in total %Ln exported positions.", nullptr, samplePoints));
    }
    ui->infoLabel->setText(infoText);
}

void BasicFlightExportDialog::frenchConnection() noexcept
{
    connect(ui->fileSelectionPushButton, &QPushButton::clicked,
            this, &BasicFlightExportDialog::onFileSelectionButtonClicked);
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &BasicFlightExportDialog::onFilePathChanged);
    connect(ui->formationExportComboBox, &QComboBox::currentIndexChanged,
            this, &BasicFlightExportDialog::onFormationExportChanged);
    connect(ui->resamplingComboBox, &QComboBox::currentIndexChanged,
            this, &BasicFlightExportDialog::onResamplingOptionChanged);
    connect(ui->openExportCheckBox, &QCheckBox::toggled,
            this, &BasicFlightExportDialog::onDoOpenExportedFilesChanged);
    connect(&d->pluginSettings, &FlightExportPluginBaseSettings::changed,
            this, &BasicFlightExportDialog::updateUi);
    const QPushButton *resetButton = ui->defaultButtonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, &QPushButton::clicked,
            this, &BasicFlightExportDialog::onRestoreDefaults);
}

inline bool BasicFlightExportDialog::isExportUserAircraftOnly() const noexcept
{
    return d->pluginSettings.getFormationExport() == FlightExportPluginBaseSettings::FormationExport::UserAircraftOnly;
}

std::size_t BasicFlightExportDialog::estimateNofSamplePoints() const noexcept
{
    std::size_t nofSamplePoints = 0;
    const std::int64_t period = ui->resamplingComboBox->currentData().toInt();
    if (period != 0) {
        if (isExportUserAircraftOnly()) {
            std::int64_t duration = d->flight.getUserAircraft().getDurationMSec();
            nofSamplePoints += static_cast<std::int64_t>(std::round(static_cast<double>(duration) / static_cast<double>(period))) + 1;
        } else {
            for (const auto &aircraft : d->flight) {
                std::int64_t duration = aircraft.getDurationMSec();
                nofSamplePoints += static_cast<std::int64_t>(std::round(static_cast<double>(duration) / static_cast<double>(period))) + 1;
            }
        }
    } else {
        // Count the actual position sample points
        if (isExportUserAircraftOnly()) {
            nofSamplePoints += d->flight.getUserAircraft().getPosition().count();
        } else {
            for (const auto &aircraft : d->flight) {
                nofSamplePoints += aircraft.getPosition().count();
            }
        }
    }
    return nofSamplePoints;
}

// PRIVATE SLOTS

void BasicFlightExportDialog::updateUi() noexcept
{
    const QString filePath = ui->filePathLineEdit->text();
    QFileInfo fileInfo(filePath);
    QFile file(fileInfo.absolutePath());
    d->exportButton->setEnabled(file.exists());

    const SampleRate::ResamplingPeriod resamplingPeriod = d->pluginSettings.getResamplingPeriod();
    int currentIndex {0};
    int indexCount = ui->resamplingComboBox->count();
    while (currentIndex < indexCount &&
           static_cast<SampleRate::ResamplingPeriod>(ui->resamplingComboBox->itemData(currentIndex).toInt()) != resamplingPeriod) {
        ++currentIndex;
    }
    if (currentIndex < indexCount) {
        ui->resamplingComboBox->setCurrentIndex(currentIndex);
    } else if (indexCount > 0) {
        // Default setting not supported -> select the first available option
        ui->resamplingComboBox->setCurrentIndex(0);
        onResamplingOptionChanged();
    }

    const FlightExportPluginBaseSettings::FormationExport formationExport = d->pluginSettings.getFormationExport();
    currentIndex = 0;
    indexCount = ui->formationExportComboBox->count();
    while (currentIndex < indexCount &&
           static_cast<FlightExportPluginBaseSettings::FormationExport>(ui->formationExportComboBox->itemData(currentIndex).toInt()) != formationExport) {
        ++currentIndex;
    }
    if (currentIndex < indexCount) {
        ui->formationExportComboBox->setCurrentIndex(currentIndex);
    } else if (indexCount > 0) {
        // Default setting not supported -> select the first available option
        ui->formationExportComboBox->setCurrentIndex(0);
        onFormationExportChanged();
    }

    switch (formationExport) {
    case FlightExportPluginBaseSettings::FormationExport::UserAircraftOnly:
        ui->formationExportComboBox->setToolTip(tr("Only the currently selected user aircraft is exported."));
        break;
    case FlightExportPluginBaseSettings::FormationExport::AllAircraftOneFile:
        ui->formationExportComboBox->setToolTip(tr("All aircraft are exported, into a single file (if supported by the format; otherwise separate files)."));
        break;
    case FlightExportPluginBaseSettings::FormationExport::AllAircraftSeparateFiles:
        ui->formationExportComboBox->setToolTip(tr("All aircraft are exported, into separate files."));
        break;
    }

    ui->openExportCheckBox->setChecked(d->pluginSettings.isOpenExportedFilesEnabled());

    updateDataGroupBox();
}

void BasicFlightExportDialog::onFileSelectionButtonClicked() noexcept
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export File"), ui->filePathLineEdit->text(), d->fileFilter);
    if (!filePath.isEmpty()) {
        ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
        d->fileDialogSelectedFile = true;
    }
    updateUi();
}

void BasicFlightExportDialog::onFilePathChanged()
{
    d->fileDialogSelectedFile = false;
    updateUi();
}

void BasicFlightExportDialog::onFormationExportChanged() noexcept
{
    d->pluginSettings.setFormationExport(static_cast<FlightExportPluginBaseSettings::FormationExport>(ui->formationExportComboBox->currentData().toInt()));
}

void BasicFlightExportDialog::onResamplingOptionChanged() noexcept
{
    // TODO REMOVE ME
    qDebug() << "BasicFlightExportDialog::onResamplingOptionChanged:" << ui->resamplingComboBox->currentData().toInt();
    d->pluginSettings.setResamplingPeriod(static_cast<SampleRate::ResamplingPeriod>(ui->resamplingComboBox->currentData().toInt()));
}

void BasicFlightExportDialog::onDoOpenExportedFilesChanged(bool enable) noexcept
{
    d->pluginSettings.setOpenExportedFilesEnabled(enable);
}

void BasicFlightExportDialog::onRestoreDefaults() noexcept
{
    d->pluginSettings.restoreDefaults();
}
