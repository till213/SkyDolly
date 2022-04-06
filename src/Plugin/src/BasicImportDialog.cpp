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
#include "BasicImportDialog.h"
#include "ui_BasicImportDialog.h"

#include <memory>

#include <QDialog>
#include <QString>
#include <QFileDialog>
#include <QWidget>
#include <QPushButton>
#include <QCompleter>

#include "../../Kernel/src/Settings.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/AircraftType.h"
#include "../../Model/src/Logbook.h"
#include "../../Model/src/Flight.h"
#include "../../Persistence/src/Service/AircraftTypeService.h"
#include "../../Kernel/src/Settings.h"
#include "ImportPluginBaseSettings.h"

namespace
{
    constexpr bool DefaultAddToFlight = false;
}

class BasicImportDialogPrivate
{
public:
    BasicImportDialogPrivate(const QString &theFileFilter, ImportPluginBaseSettings &theSettings) noexcept
        : aircraftTypeService(std::make_unique<AircraftTypeService>()),
          fileFilter(theFileFilter),
          settings(theSettings),
          importButton(nullptr),
          optionWidget(nullptr)
    {}

    std::unique_ptr<AircraftTypeService> aircraftTypeService;
    QString fileFilter;
    ImportPluginBaseSettings &settings;
    QPushButton *importButton;
    QWidget *optionWidget;
};

// PUBLIC

BasicImportDialog::BasicImportDialog(const QString &fileExtension, ImportPluginBaseSettings &settings, QWidget *parent) :
    QDialog(parent),
    ui(std::make_unique<Ui::BasicImportDialog>()),
    d(std::make_unique<BasicImportDialogPrivate>(fileExtension, settings))
{
    ui->setupUi(this);    
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("BasicImportDialog::BasicImportDialog: CREATED");
#endif
}

BasicImportDialog::~BasicImportDialog()
{
#ifdef DEBUG
    qDebug("BasicImportDialog::~BasicImportDialog: DELETED");
#endif
}

QString BasicImportDialog::getSelectedFilePath() const noexcept
{
    return ui->filePathLineEdit->text();
}

bool BasicImportDialog::getSelectedAircraftType(AircraftType &aircraftType) const noexcept
{
    return d->aircraftTypeService->getByType(ui->aircraftSelectionComboBox->currentText(), aircraftType);
}

QString BasicImportDialog::getFileFilter() const noexcept
{
    return d->fileFilter;
}

void BasicImportDialog::setFileFilter(const QString &extension) noexcept
{
    d->fileFilter = extension;
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
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    QString type = flight.getUserAircraftConst().getAircraftInfoConst().aircraftType.type;
    if (type.isEmpty()) {
        type = settings.getImportAircraftType();
    }
    if (!type.isEmpty()) {
        ui->aircraftSelectionComboBox->setCurrentText(type);
    }
    ui->addToFlightCheckBox->setChecked(::DefaultAddToFlight);
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
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &BasicImportDialog::updateUi);
    connect(ui->fileSelectionPushButton, &QPushButton::clicked,
            this, &BasicImportDialog::onFileSelectionChanged);
    connect(ui->aircraftSelectionComboBox, &QComboBox::currentTextChanged,
            this, &BasicImportDialog::updateUi);    
    connect(ui->addToFlightCheckBox, &QCheckBox::toggled,
            this, &BasicImportDialog::onAddToExistingFlightChanged);
    connect(&d->settings, &ImportPluginBaseSettings::baseSettingsChanged,
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
    const QString filePath = ui->filePathLineEdit->text();
    QFile file(filePath);
    const QString type = ui->aircraftSelectionComboBox->currentText();
    const bool aircraftTypeExists = !type.isEmpty() && d->aircraftTypeService->exists(type);
    const bool enabled = file.exists() && aircraftTypeExists;
    d->importButton->setEnabled(enabled);

    ui->addToFlightCheckBox->setChecked(d->settings.isAddToFlightEnabled());
}

void BasicImportDialog::onFileSelectionChanged() noexcept
{
    // Start with the last export path
    QString exportPath = Settings::getInstance().getExportPath();

    const QString filePath = QFileDialog::getOpenFileName(this, tr("Import file..."), exportPath, d->fileFilter);
    if (!filePath.isEmpty()) {
        ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
    }
}

void BasicImportDialog::onAddToExistingFlightChanged(bool enable) noexcept
{
    d->settings.setAddToFlightEnabled(enable);
}

void BasicImportDialog::onRestoreDefaults() noexcept
{
    initBasicUi();
    emit restoreDefaultOptions();
}

void BasicImportDialog::onAccepted() noexcept
{
    const QString type = ui->aircraftSelectionComboBox->currentText();
    Settings::getInstance().setImportAircraftType(type);
}
