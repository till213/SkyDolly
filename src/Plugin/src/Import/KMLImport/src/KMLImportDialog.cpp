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
#include <QCoreApplication>
#include <QString>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>

#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/AircraftType.h"
#include "../../../../../Persistence/src/Service/AircraftTypeService.h"
#include "KMLImportDialog.h"
#include "KMLImportSettings.h"
#include "ui_KMLImportDialog.h"

class KMLImportDialogPrivate
{
public:
    KMLImportDialogPrivate(KMLImportSettings &theImportSettings) noexcept
        : aircraftTypeService(std::make_unique<AircraftTypeService>()),
          importSettings(theImportSettings),
          importButton(nullptr)
    {}

    std::unique_ptr<AircraftTypeService> aircraftTypeService;
    KMLImportSettings &importSettings;
    QPushButton *importButton;
};

// PUBLIC

KMLImportDialog::KMLImportDialog(KMLImportSettings &importSettings, QWidget *parent) noexcept
    : QDialog(parent),
      ui(new Ui::KMLImportDialog),
      d(std::make_unique<KMLImportDialogPrivate>(importSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

KMLImportDialog::~KMLImportDialog() noexcept
{
    delete ui;
}

QString KMLImportDialog::getSelectedFilePath() const noexcept
{
    return ui->filePathLineEdit->text();
}

bool KMLImportDialog::getSelectedAircraftType(AircraftType &aircraftType) const noexcept
{
    return d->aircraftTypeService->getByType(ui->aircraftSelectionComboBox->currentText(), aircraftType);
}

bool KMLImportDialog::isAddToFlightEnabled() const noexcept
{
    return ui->addToFlightCheckBox->isChecked();
}

// PRIVATE

void KMLImportDialog::frenchConnection() noexcept
{
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &KMLImportDialog::updateUi);

    QPushButton *resetButton = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(resetButton, &QPushButton::clicked,
            this, &KMLImportDialog::restoreDefaults);
}

void KMLImportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->importButton = ui->buttonBox->addButton(tr("Import"), QDialogButtonBox::AcceptRole);
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    QString type = flight.getUserAircraftConst().getAircraftInfoConst().aircraftType.type;
    if (!type.isEmpty()) {
        ui->aircraftSelectionComboBox->setCurrentText(type);
    }

    initOptionUi();
}

void KMLImportDialog::initOptionUi() noexcept
{
    ui->formatComboBox->addItem("FlightAware.com", Enum::toUnderlyingType(KMLImportSettings::Format::FlightAware));
    ui->formatComboBox->addItem("FlightRadar24.com", Enum::toUnderlyingType(KMLImportSettings::Format::FlightRadar24));
}

void KMLImportDialog::updateOptionUi() noexcept
{
    int currentIndex = 0;
    while (currentIndex < ui->formatComboBox->count() &&
           static_cast<KMLImportSettings::Format>(ui->formatComboBox->itemData(currentIndex).toInt()) != d->importSettings.format) {
        ++currentIndex;
    }
    ui->formatComboBox->setCurrentIndex(currentIndex);
}

// PRIVATE SLOTS

void KMLImportDialog::on_fileSelectionPushButton_clicked() noexcept
{
    // Start with the last export path
    QString exportPath = Settings::getInstance().getExportPath();

    const QString filePath = QFileDialog::getOpenFileName(this, QCoreApplication::translate("KMLImportDialog", "Import KML"), exportPath, QString("*.KML"));
    if (!filePath.isEmpty()) {
        ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
    }
}

void KMLImportDialog::on_formatComboBox_activated([[maybe_unused]]int index) noexcept
{
    d->importSettings.format = static_cast<KMLImportSettings::Format>(ui->formatComboBox->currentData().toInt());
}

void KMLImportDialog::updateUi() noexcept
{
    const QString filePath = ui->filePathLineEdit->text();
    QFile file(filePath);
    d->importButton->setEnabled(file.exists());

    updateOptionUi();
}

void KMLImportDialog::restoreDefaults() noexcept
{
    d->importSettings.restoreDefaults();
    updateUi();
}
