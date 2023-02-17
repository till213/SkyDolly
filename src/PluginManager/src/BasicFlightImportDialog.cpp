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

#include <QDialog>
#include <QString>
#include <QStringBuilder>
#include <QFileDialog>
#include <QComboBox>
#include <QWidget>
#include <QPushButton>
#include <QCompleter>

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Model/Aircraft.h>
#include <Model/AircraftType.h>
#include <Model/Flight.h>
#include <Persistence/Service/AircraftTypeService.h>
#include "FlightImportPluginBaseSettings.h"
#include "BasicFlightImportDialog.h"
#include "ui_BasicFlightImportDialog.h"

struct BasicFlightImportDialogPrivate
{
    BasicFlightImportDialogPrivate(const Flight &theFlight, QString fileFilter, FlightImportPluginBaseSettings &pluginSettings) noexcept
        : flight(theFlight),
          fileFilter(std::move(fileFilter)),
          pluginSettings(pluginSettings)
    {}

    const Flight &flight;
    std::unique_ptr<AircraftTypeService> aircraftTypeService {std::make_unique<AircraftTypeService>()};
    QString fileFilter;
    FlightImportPluginBaseSettings &pluginSettings;
    QPushButton *importButton {nullptr};
    QWidget *optionWidget {nullptr};
};

// PUBLIC

BasicFlightImportDialog::BasicFlightImportDialog(const Flight &flight, const QString &fileFilter, FlightImportPluginBaseSettings &pluginSettings, QWidget *parent) noexcept
    : QDialog(parent),
      ui(std::make_unique<Ui::BasicFlightImportDialog>()),
      d(std::make_unique<BasicFlightImportDialogPrivate>(flight, fileFilter, pluginSettings))
{
        ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

BasicFlightImportDialog::~BasicFlightImportDialog() = default;

QString BasicFlightImportDialog::getSelectedPath() const noexcept
{
    return ui->pathLineEdit->text();
}

AircraftType BasicFlightImportDialog::getSelectedAircraftType(bool *ok) const noexcept
{
    return d->aircraftTypeService->getByType(ui->aircraftSelectionComboBox->currentText(), ok);
}

QString BasicFlightImportDialog::getFileFilter() const noexcept
{
    return d->fileFilter;
}

void BasicFlightImportDialog::setFileFilter(const QString &fileFilter) noexcept
{
    d->fileFilter = fileFilter;
}

void BasicFlightImportDialog::setOptionWidget(QWidget *widget) noexcept
{
    d->optionWidget = widget;
    initOptionUi();
}

// PRIVATE

void BasicFlightImportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->importButton = ui->defaultButtonBox->addButton(tr("&Import"), QDialogButtonBox::AcceptRole);
    initBasicUi();
    initOptionUi();
}

void BasicFlightImportDialog::initBasicUi() noexcept
{
    Settings &settings = Settings::getInstance();
    ui->pathLineEdit->setText(QDir::toNativeSeparators(settings.getExportPath()));

    QString type = d->flight.getUserAircraft().getAircraftInfo().aircraftType.type;
    if (type.isEmpty()) {
        type = settings.getImportAircraftType();
    }
    if (!type.isEmpty()) {
        ui->aircraftSelectionComboBox->setCurrentText(type);
    }

    // Import aircraft mode
    ui->aircraftImportModeComboBox->addItem(tr("Add to current flight"), Enum::underly(FlightImportPluginBaseSettings::AircraftImportMode::AddToCurrentFlight));
    ui->aircraftImportModeComboBox->addItem(tr("Add to new flight"), Enum::underly(FlightImportPluginBaseSettings::AircraftImportMode::AddToNewFlight));

    // Time offset synchronisation
    ui->timeOffsetSyncComboBox->addItem(tr("None"), Enum::underly(SkyMath::TimeOffsetSync::None));
    ui->timeOffsetSyncComboBox->addItem(tr("Time only"), Enum::underly(SkyMath::TimeOffsetSync::TimeOnly));
    ui->timeOffsetSyncComboBox->addItem(tr("Date and time"), Enum::underly(SkyMath::TimeOffsetSync::DateAndTime));

    if (d->pluginSettings.requiresAircraftSelection()) {
        ui->aircraftSelectionComboBox->setEnabled(true);
        ui->aircraftSelectionComboBox->setToolTip(tr("Select the aircraft type for each imported aircraft."));
    } else {
        ui->aircraftSelectionComboBox->setEnabled(false);
        ui->aircraftSelectionComboBox->setToolTip(tr("The aircraft type is defined by the file format (%1).").arg(d->fileFilter));
    }
}

void BasicFlightImportDialog::initOptionUi() noexcept
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

void BasicFlightImportDialog::frenchConnection() noexcept
{
    connect(ui->pathLineEdit, &QLineEdit::textChanged,
            this, &BasicFlightImportDialog::updateUi);
    connect(ui->fileSelectionPushButton, &QPushButton::clicked,
            this, &BasicFlightImportDialog::onFileSelectionChanged);
    connect(ui->aircraftSelectionComboBox, &QComboBox::currentTextChanged,
            this, &BasicFlightImportDialog::updateUi);
    connect(ui->importDirectoryCheckBox, &QCheckBox::toggled,
            this, &BasicFlightImportDialog::onImportDirectoryChanged);
    connect(ui->aircraftImportModeComboBox, &QComboBox::currentIndexChanged,
            this, &BasicFlightImportDialog::onAircraftImportModeChanged);
    connect(ui->timeOffsetSyncComboBox, &QComboBox::currentIndexChanged,
            this, &BasicFlightImportDialog::onTimeOffsetSyncChanged);
    connect(&d->pluginSettings, &FlightImportPluginBaseSettings::baseSettingsChanged,
            this, &BasicFlightImportDialog::updateUi);
    const QPushButton *resetButton = ui->defaultButtonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, &QPushButton::clicked,
            this, &BasicFlightImportDialog::onRestoreDefaults);
    connect(ui->defaultButtonBox, &QDialogButtonBox::accepted,
            this, &BasicFlightImportDialog::onAccepted);
}

// PRIVATE SLOTS

void BasicFlightImportDialog::updateUi() noexcept
{
    constexpr int CreateSeparateFlightsIndex = 2;
    const QString filePath = ui->pathLineEdit->text();
    QFileInfo fileInfo {filePath};

    bool fileExists {false};
    if (d->pluginSettings.isImportDirectoryEnabled()) {
        fileExists = fileInfo.isDir() && fileInfo.exists();
    } else {
        fileExists = fileInfo.isFile() && fileInfo.exists();
    }

    const QString type = ui->aircraftSelectionComboBox->currentText();
    const bool aircraftTypeExists = !type.isEmpty() && d->aircraftTypeService->exists(type);
    const bool enabled = fileExists && aircraftTypeExists;
    d->importButton->setEnabled(enabled);

    const bool importDirectory = d->pluginSettings.isImportDirectoryEnabled();
    ui->importDirectoryCheckBox->setChecked(importDirectory);
    if (importDirectory) {
        const QString currentPath = ui->pathLineEdit->text();
        if (!currentPath.isEmpty()) {
            QFileInfo fileInfo {currentPath};
            if (fileInfo.isFile()) {
                ui->pathLineEdit->setText(QDir::toNativeSeparators(fileInfo.absolutePath()));
            }
        }
        if (ui->aircraftImportModeComboBox->count() <= CreateSeparateFlightsIndex) {
            ui->aircraftImportModeComboBox->addItem(tr("Separate flights"), Enum::underly(FlightImportPluginBaseSettings::AircraftImportMode::SeparateFlights));
        }
    } else if (ui->aircraftImportModeComboBox->count() == CreateSeparateFlightsIndex + 1) {
        ui->aircraftImportModeComboBox->removeItem(ui->aircraftImportModeComboBox->count() - 1);
    }

    const FlightImportPluginBaseSettings::AircraftImportMode aircraftImportMode = d->pluginSettings.getAircraftImportMode();
    int indexCount = ui->aircraftImportModeComboBox->count();
    int currentIndex {0};
    while (currentIndex < indexCount &&
           static_cast<FlightImportPluginBaseSettings::AircraftImportMode>(ui->aircraftImportModeComboBox->itemData(currentIndex).toInt()) != aircraftImportMode) {
        ++currentIndex;
    }
    if (currentIndex < indexCount) {
        ui->aircraftImportModeComboBox->setCurrentIndex(currentIndex);
    }

    switch (aircraftImportMode) {
    case FlightImportPluginBaseSettings::AircraftImportMode::AddToCurrentFlight:
        ui->aircraftImportModeComboBox->setToolTip(tr("Add all imported aircraft to current flight."));
        break;
    case FlightImportPluginBaseSettings::AircraftImportMode::AddToNewFlight:
        ui->aircraftImportModeComboBox->setToolTip(tr("Add all imported aircraft to newly created flight."));
        break;
    case FlightImportPluginBaseSettings::AircraftImportMode::SeparateFlights:
        ui->aircraftImportModeComboBox->setToolTip(tr("Create separate flights for each imported file."));
        break;
    }

    const SkyMath::TimeOffsetSync timeOffsetSync = d->pluginSettings.getTimeOffsetSync();
    indexCount = ui->timeOffsetSyncComboBox->count();
    currentIndex = 0;
    while (currentIndex < indexCount &&
           static_cast<SkyMath::TimeOffsetSync>(ui->timeOffsetSyncComboBox->itemData(currentIndex).toInt()) != timeOffsetSync) {
        ++currentIndex;
    }
    if (currentIndex < indexCount) {
        ui->timeOffsetSyncComboBox->setCurrentIndex(currentIndex);
    }

    switch (timeOffsetSync) {
    case SkyMath::TimeOffsetSync::None:
        ui->timeOffsetSyncComboBox->setToolTip(tr("No time offset is applied to any timestamps."));
        break;
    case SkyMath::TimeOffsetSync::DateAndTime:
        ui->timeOffsetSyncComboBox->setToolTip(tr("Timestamps of newly imported aircraft are being offset based on the difference between the current and the newly imported flight, taking both recording date and time into account."));
        break;
    case SkyMath::TimeOffsetSync::TimeOnly:
        ui->timeOffsetSyncComboBox->setToolTip(tr("Timestamps of newly imported aircraft are being offset based on the difference between the current and the newly imported flight, taking only the recording time into account (ignoring the recording date)."));
        break;
    }
}

void BasicFlightImportDialog::onFileSelectionChanged() noexcept
{
    // Start with the last export path
    QString exportPath;
    const QString currentFilePath = ui->pathLineEdit->text();
    if (currentFilePath.isEmpty()) {
        exportPath = Settings::getInstance().getExportPath();
    } else {
        QFileInfo fileInfo {currentFilePath};
        if (fileInfo.isDir()) {
            exportPath = fileInfo.absoluteFilePath();
        } else {
            exportPath = QFileInfo(currentFilePath).absolutePath();
        }
        if (!QFileInfo::exists(exportPath)) {
            exportPath = Settings::getInstance().getExportPath();
        }
    }

    QString filePath;
    if (d->pluginSettings.isImportDirectoryEnabled()) {
        filePath = QFileDialog::getExistingDirectory(this, tr("Import Directory"), exportPath);
    } else {
        filePath = QFileDialog::getOpenFileName(this, tr("Import File"), exportPath, d->fileFilter);
    }
    if (!filePath.isEmpty()) {
        ui->pathLineEdit->setText(QDir::toNativeSeparators(filePath));
    }
}

void BasicFlightImportDialog::onImportDirectoryChanged(bool enable) noexcept
{
    d->pluginSettings.setImportDirectoryEnabled(enable);
}

void BasicFlightImportDialog::onAircraftImportModeChanged() noexcept
{
    d->pluginSettings.setAircraftImportMode(static_cast<FlightImportPluginBaseSettings::AircraftImportMode>(ui->aircraftImportModeComboBox->currentData().toInt()));
}

void BasicFlightImportDialog::onTimeOffsetSyncChanged() noexcept
{
    d->pluginSettings.setTimeOffsetSync(static_cast<SkyMath::TimeOffsetSync>(ui->timeOffsetSyncComboBox->currentData().toInt()));
}

void BasicFlightImportDialog::onRestoreDefaults() noexcept
{
    d->pluginSettings.restoreDefaults();
}

void BasicFlightImportDialog::onAccepted() noexcept
{
    const QString type = ui->aircraftSelectionComboBox->currentText();
    Settings::getInstance().setImportAircraftType(type);
}
