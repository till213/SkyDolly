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

#include <QDialog>
#include <QString>
#include <QStringBuilder>
#include <QFileDialog>
#include <QWidget>
#include <QPushButton>
#include <QCompleter>

#include <Kernel/Settings.h>
#include <Model/Aircraft.h>
#include <Model/AircraftType.h>
#include <Model/Flight.h>
#include <Persistence/Service/AircraftTypeService.h>
#include <Kernel/Settings.h>
#include "ImportPluginBaseSettings.h"
#include "BasicImportDialog.h"
#include "ui_BasicImportDialog.h"

struct BasicImportDialogPrivate
{
    BasicImportDialogPrivate(const Flight &theFlight, const QString &theFileFilter, ImportPluginBaseSettings &pluginSettings) noexcept
        : flight(theFlight),
          aircraftTypeService(std::make_unique<AircraftTypeService>()),
          fileFilter(theFileFilter),
          pluginSettings(pluginSettings),
          importButton(nullptr),
          optionWidget(nullptr)
    {}

    const Flight &flight;
    std::unique_ptr<AircraftTypeService> aircraftTypeService;
    QString fileFilter;
    ImportPluginBaseSettings &pluginSettings;
    QPushButton *importButton;
    QWidget *optionWidget;
};

// PUBLIC

BasicImportDialog::BasicImportDialog(const Flight &flight, const QString &fileSuffix, ImportPluginBaseSettings &pluginSettings, QWidget *parent) noexcept
    : QDialog(parent),
      ui(std::make_unique<Ui::BasicImportDialog>()),
      d(std::make_unique<BasicImportDialogPrivate>(flight, fileSuffix, pluginSettings))
{
    ui->setupUi(this);    
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("BasicImportDialog::BasicImportDialog: CREATED");
#endif
}

BasicImportDialog::~BasicImportDialog() noexcept
{
#ifdef DEBUG
    qDebug("BasicImportDialog::~BasicImportDialog: DELETED");
#endif
}

QString BasicImportDialog::getSelectedPath() const noexcept
{
    return ui->pathLineEdit->text();
}

bool BasicImportDialog::getSelectedAircraftType(AircraftType &aircraftType) const noexcept
{
    return d->aircraftTypeService->getByType(ui->aircraftSelectionComboBox->currentText(), aircraftType);
}

QString BasicImportDialog::getFileFilter() const noexcept
{
    return d->fileFilter;
}

void BasicImportDialog::setFileFilter(const QString &fileFilter) noexcept
{
    d->fileFilter = fileFilter;
}

void BasicImportDialog::setOptionWidget(QWidget *widget) noexcept
{
    d->optionWidget = widget;
    initOptionUi();
}

// PRIVATE

void BasicImportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->importButton = ui->defaultButtonBox->addButton(tr("&Import"), QDialogButtonBox::AcceptRole);
    initBasicUi();
    initOptionUi();
}

void BasicImportDialog::initBasicUi() noexcept
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
    ui->importDirectoryCheckBox->setChecked(d->pluginSettings.isImportDirectoryEnabled());
    ui->addToFlightCheckBox->setChecked(d->pluginSettings.isAddToFlightEnabled());
}

void BasicImportDialog::initOptionUi() noexcept
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

void BasicImportDialog::frenchConnection() noexcept
{
    connect(ui->pathLineEdit, &QLineEdit::textChanged,
            this, &BasicImportDialog::updateUi);
    connect(ui->fileSelectionPushButton, &QPushButton::clicked,
            this, &BasicImportDialog::onFileSelectionChanged);
    connect(ui->aircraftSelectionComboBox, &QComboBox::currentTextChanged,
            this, &BasicImportDialog::updateUi);
    connect(ui->importDirectoryCheckBox, &QCheckBox::toggled,
            this, &BasicImportDialog::onImportDirectoryChanged);
    connect(ui->addToFlightCheckBox, &QCheckBox::toggled,
            this, &BasicImportDialog::onAddToExistingFlightChanged);
    connect(&d->pluginSettings, &ImportPluginBaseSettings::baseSettingsChanged,
            this, &BasicImportDialog::updateUi);
    const QPushButton *resetButton = ui->defaultButtonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, &QPushButton::clicked,
            this, &BasicImportDialog::onRestoreDefaults);
    connect(ui->defaultButtonBox, &QDialogButtonBox::accepted,
            this, &BasicImportDialog::onAccepted);
}

// PRIVATE SLOTS

void BasicImportDialog::updateUi() noexcept
{
    const QString filePath = ui->pathLineEdit->text();
    QFileInfo fileInfo {filePath};

    bool fileExists;
    if (d->pluginSettings.isImportDirectoryEnabled()) {
        fileExists = fileInfo.isDir() && fileInfo.exists();
    } else {
        fileExists = fileInfo.isFile() && fileInfo.exists();
    }

    const QString type = ui->aircraftSelectionComboBox->currentText();
    const bool aircraftTypeExists = !type.isEmpty() && d->aircraftTypeService->exists(type);
    const bool enabled = fileExists && aircraftTypeExists;
    d->importButton->setEnabled(enabled);

    if (d->pluginSettings.isImportDirectoryEnabled()) {
        ui->importDirectoryCheckBox->setChecked(true);
        const QString currentPath = ui->pathLineEdit->text();
        if (!currentPath.isEmpty()) {
            QFileInfo fileInfo {currentPath};
            if (fileInfo.isFile()) {
                ui->pathLineEdit->setText(QDir::toNativeSeparators(fileInfo.absolutePath()));
            }
        }
        ui->addToFlightCheckBox->setText(tr("Add all aircraft to same new flight"));
        ui->addToFlightCheckBox->setToolTip(tr("When checked then all aircraft are added to the same newly created flight. Otherwise a new flight is created for each imported file."));
    } else {
        ui->importDirectoryCheckBox->setChecked(false);
        ui->addToFlightCheckBox->setText(tr("Add aircraft to current flight"));
        ui->addToFlightCheckBox->setToolTip(tr("When checked then the imported aircraft is added to the currently loaded flight. Otherwise a new flight is created."));
    }
    ui->addToFlightCheckBox->setChecked(d->pluginSettings.isAddToFlightEnabled());
}

void BasicImportDialog::onFileSelectionChanged() noexcept
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

void BasicImportDialog::onImportDirectoryChanged(bool enable) noexcept
{
    d->pluginSettings.setImportDirectoryEnabled(enable);
}

void BasicImportDialog::onAddToExistingFlightChanged(bool enable) noexcept
{
    d->pluginSettings.setAddToFlightEnabled(enable);
}

void BasicImportDialog::onRestoreDefaults() noexcept
{
    d->pluginSettings.restoreDefaults();
}

void BasicImportDialog::onAccepted() noexcept
{
    const QString type = ui->aircraftSelectionComboBox->currentText();
    Settings::getInstance().setImportAircraftType(type);
}
