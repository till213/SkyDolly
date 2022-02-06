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
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/AircraftType.h"
#include "../../../../../Persistence/src/Service/AircraftTypeService.h"
#include "IGCImportDialog.h"
#include "ui_IGCImportDialog.h"

class IGCImportDialogPrivate
{
public:
    IGCImportDialogPrivate() noexcept
        : aircraftTypeService(std::make_unique<AircraftTypeService>())
    {}

    std::unique_ptr<AircraftTypeService> aircraftTypeService;
    QPushButton *importButton;
};

// PUBLIC

IGCImportDialog::IGCImportDialog(QWidget *parent) noexcept
    : QDialog(parent),
      ui(new Ui::IGCImportDialog),
      d(std::make_unique<IGCImportDialogPrivate>())
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

IGCImportDialog::~IGCImportDialog() noexcept
{
    delete ui;
}

QString IGCImportDialog::getSelectedFilePath() const noexcept
{
    return ui->filePathLineEdit->text();
}

bool IGCImportDialog::getSelectedAircraftType(AircraftType &aircraftType) const noexcept
{
    return d->aircraftTypeService->getByType(ui->aircraftSelectionComboBox->currentText(), aircraftType);
}

bool IGCImportDialog::isAddToFlightEnabled() const noexcept
{
    return ui->addToFlightCheckBox->isChecked();
}

// PRIVATE

void IGCImportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->importButton = ui->buttonBox->addButton(tr("Import"), QDialogButtonBox::AcceptRole);
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    QString type = flight.getUserAircraftConst().getAircraftInfoConst().aircraftType.type;
    if (!type.isEmpty()) {
        ui->aircraftSelectionComboBox->setCurrentText(type);
    }
}

void IGCImportDialog::frenchConnection() noexcept
{
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &IGCImportDialog::updateUi);
}

// PRIVATE SLOTS

void IGCImportDialog::on_fileSelectionPushButton_clicked() noexcept
{
    // Start with the last export path
    QString exportPath = Settings::getInstance().getExportPath();

    const QString filePath = QFileDialog::getOpenFileName(this, QCoreApplication::translate("IGCImportDialog", "Import IGC"), exportPath, QString("*.IGC"));
    if (!filePath.isEmpty()) {
        ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
    }
    updateUi();
}

void IGCImportDialog::updateUi() noexcept
{
    const QString filePath = ui->filePathLineEdit->text();
    QFile file(filePath);
    d->importButton->setEnabled(file.exists());
}
