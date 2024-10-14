/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <QStringLiteral>
#include <QStringBuilder>
#include <QFileDialog>
#include <QWidget>
#include <QPushButton>
#include <QCompleter>

#include <Kernel/Settings.h>
#include <Model/Aircraft.h>
#include <Model/AircraftType.h>
#include <Model/Location.h>
#include <Persistence/Service/AircraftTypeService.h>
#include "Kernel/Enum.h"
#include <Location/LocationImportPluginBaseSettings.h>
#include <Location/BasicLocationImportDialog.h>
#include "ui_BasicLocationImportDialog.h"

struct BasicLocationImportDialogPrivate
{
    BasicLocationImportDialogPrivate(QString fileFilter, LocationImportPluginBaseSettings &pluginSettings) noexcept
        : fileFilter {std::move(fileFilter)},
          pluginSettings {pluginSettings}
    {}

    std::unique_ptr<AircraftTypeService> aircraftTypeService {std::make_unique<AircraftTypeService>()};
    QString fileFilter;
    LocationImportPluginBaseSettings &pluginSettings;
    QPushButton *importButton {nullptr};
    QWidget *optionWidget {nullptr};
};

// PUBLIC

BasicLocationImportDialog::BasicLocationImportDialog(QString fileFilter, LocationImportPluginBaseSettings &pluginSettings, QWidget *parent) noexcept
    : QDialog {parent},
      ui {std::make_unique<Ui::BasicLocationImportDialog>()},
      d {std::make_unique<BasicLocationImportDialogPrivate>(std::move(fileFilter), pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

BasicLocationImportDialog::~BasicLocationImportDialog() = default;

QString BasicLocationImportDialog::getSelectedPath() const noexcept
{
    return ui->pathLineEdit->text();
}

QString BasicLocationImportDialog::getFileFilter() const noexcept
{
    return d->fileFilter;
}

void BasicLocationImportDialog::setFileFilter(QString fileFilter) noexcept
{
    d->fileFilter = std::move(fileFilter);
}

void BasicLocationImportDialog::setOptionWidget(QWidget *widget) noexcept
{
    d->optionWidget = widget;
    initOptionUi();
}

// PRIVATE

void BasicLocationImportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->importButton = ui->defaultButtonBox->addButton(tr("&Import"), QDialogButtonBox::AcceptRole);
    initBasicUi();
    initOptionUi();
}

void BasicLocationImportDialog::initBasicUi() noexcept
{
    const auto &settings = Settings::getInstance();
    ui->pathLineEdit->setText(QDir::toNativeSeparators(settings.getExportPath()));
    ui->importModeComboBox->setToolTip(
QStringLiteral("<html>") %
"<head/>" %
"<body><p>" %
tr("Defines how existing locations are modified:") % "<br/><br/>" %
tr("• Skip: existing locations are left unmodified (current location to be imported is skipped)") % "<br/>" %
tr("• Update: existing locations are modified with the newly imported location") % "<br/>" %
tr("• Insert: new locations are added (existing locations are left unmodified)") % "</p>"
"</body></html>");

    ui->importModeComboBox->addItem(tr("Skip"), Enum::underly(LocationService::Mode::Skip));
    ui->importModeComboBox->addItem(tr("Update"), Enum::underly(LocationService::Mode::Update));
    ui->importModeComboBox->addItem(tr("Insert"), Enum::underly(LocationService::Mode::Insert));
}

void BasicLocationImportDialog::initOptionUi() noexcept
{
    if (d->optionWidget != nullptr) {
        ui->optionGroupBox->setHidden(false);
        std::unique_ptr<QLayout> layout {ui->optionGroupBox->layout()};
        // Any previously existing layout is deleted first, which is what we want
        layout = std::make_unique<QVBoxLayout>();
        layout->addWidget(d->optionWidget);
        // Transfer ownership of the layout to the optionGroupBox
        ui->optionGroupBox->setLayout(layout.release());
    } else {
        ui->optionGroupBox->setHidden(true);
    }
}

void BasicLocationImportDialog::frenchConnection() noexcept
{
    connect(ui->pathLineEdit, &QLineEdit::textChanged,
            this, &BasicLocationImportDialog::updateUi);
    connect(ui->fileSelectionPushButton, &QPushButton::clicked,
            this, &BasicLocationImportDialog::onFileSelectionChanged);
    connect(ui->importDirectoryCheckBox, &QCheckBox::toggled,
            this, &BasicLocationImportDialog::onImportDirectoryChanged);
    connect(ui->importModeComboBox, &QComboBox::currentIndexChanged,
            this, &BasicLocationImportDialog::onImportModeChanged);
    connect(ui->distanceSpinBox, &QDoubleSpinBox::valueChanged,
            this, &BasicLocationImportDialog::onDistanceValueChanged);
    connect(&d->pluginSettings, &LocationImportPluginBaseSettings::changed,
            this, &BasicLocationImportDialog::updateUi);
    const QPushButton *resetButton = ui->defaultButtonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, &QPushButton::clicked,
            this, &BasicLocationImportDialog::onRestoreDefaults);
}

// PRIVATE SLOTS

void BasicLocationImportDialog::updateUi() noexcept
{
    const QString filePath = ui->pathLineEdit->text();
    const QFileInfo fileInfo {filePath};

    bool fileExists {false};
    if (d->pluginSettings.isImportDirectoryEnabled()) {
        fileExists = fileInfo.isDir() && fileInfo.exists();
    } else {
        fileExists = fileInfo.isFile() && fileInfo.exists();
    }

    const LocationService::Mode importMode = d->pluginSettings.getImportMode();
    int currentIndex = 0;
    while (currentIndex < ui->importModeComboBox->count() &&
           static_cast<LocationService::Mode>(ui->importModeComboBox->itemData(currentIndex).toInt()) != importMode) {
        ++currentIndex;
    }
    ui->importModeComboBox->setCurrentIndex(currentIndex);
    ui->distanceSpinBox->setValue(d->pluginSettings.getNearestLocationDistanceKm());
    ui->distanceSpinBox->setEnabled(importMode != LocationService::Mode::Insert);

    const bool enabled = fileExists;
    d->importButton->setEnabled(enabled);

    if (d->pluginSettings.isImportDirectoryEnabled()) {
        ui->importDirectoryCheckBox->setChecked(true);
        const QString currentPath = ui->pathLineEdit->text();
        if (!currentPath.isEmpty()) {
            const QFileInfo fileInfo {currentPath};
            if (fileInfo.isFile()) {
                ui->pathLineEdit->setText(QDir::toNativeSeparators(fileInfo.absolutePath()));
            }
        }
    } else {
        ui->importDirectoryCheckBox->setChecked(false);
    }
}

void BasicLocationImportDialog::onFileSelectionChanged() noexcept
{
    // Start with the last export path
    QString exportPath;
    const QString currentFilePath = ui->pathLineEdit->text();
    if (currentFilePath.isEmpty()) {
        exportPath = Settings::getInstance().getExportPath();
    } else {
        const QFileInfo fileInfo {currentFilePath};
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

void BasicLocationImportDialog::onImportDirectoryChanged(bool enable) noexcept
{
    d->pluginSettings.setImportDirectoryEnabled(enable);
}

void BasicLocationImportDialog::onImportModeChanged() noexcept
{
    const auto mode = static_cast<LocationService::Mode>(ui->importModeComboBox->currentData().toInt());
    d->pluginSettings.setImportMode(mode);
}

void BasicLocationImportDialog::onDistanceValueChanged() noexcept
{
    const auto value = ui->distanceSpinBox->value();
    d->pluginSettings.setNearestLocationDistanceKm(value);
}

void BasicLocationImportDialog::onRestoreDefaults() noexcept
{
    d->pluginSettings.restoreDefaults();
}
