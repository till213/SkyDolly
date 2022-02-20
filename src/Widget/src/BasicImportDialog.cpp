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

class BasicImportDialogPrivate
{
public:
    BasicImportDialogPrivate(const QString &theFileFilter) noexcept
        : aircraftTypeService(std::make_unique<AircraftTypeService>()),
          fileFilter(theFileFilter),
          importButton(nullptr),
          optionWidget(nullptr)
    {}

    std::unique_ptr<AircraftTypeService> aircraftTypeService;
    QString fileFilter;
    QPushButton *importButton;
    QWidget *optionWidget;
};

// PUBLIC

BasicImportDialog::BasicImportDialog(const QString &fileExtension, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BasicImportDialog),
    d(std::make_unique<BasicImportDialogPrivate>(fileExtension))
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
    delete ui;
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

bool BasicImportDialog::isAddToFlightEnabled() const noexcept
{
    return ui->addToFlightCheckBox->isChecked();
}

void BasicImportDialog::setFileFilter(const QString &extension) noexcept
{
    d->fileFilter = extension;
}

QString BasicImportDialog::getFileFilter() const noexcept
{
    return d->fileFilter;
}

void BasicImportDialog::setOptionWidget(QWidget *widget) noexcept
{
    d->optionWidget = widget;
    initOptionUi();
}

// PRIVATE

void BasicImportDialog::frenchConnection() noexcept
{
    connect(ui->filePathLineEdit, &QLineEdit::textChanged,
            this, &BasicImportDialog::updateUi);
    connect(ui->fileSelectionPushButton, &QPushButton::clicked,
            this, &BasicImportDialog::onFileSelectionPushButtonClicked);
    connect(ui->aircraftSelectionComboBox, &QComboBox::currentTextChanged,
            this, &BasicImportDialog::updateUi);
}

void BasicImportDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    d->importButton = ui->defaultButtonBox->addButton(tr("Import"), QDialogButtonBox::AcceptRole);
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    QString type = flight.getUserAircraftConst().getAircraftInfoConst().aircraftType.type;
    if (!type.isEmpty()) {
        ui->aircraftSelectionComboBox->setCurrentText(type);
    }
    ui->aircraftSelectionComboBox->setEditable(true);
    ui->aircraftSelectionComboBox->setInsertPolicy(QComboBox::NoInsert);

    initOptionUi();
}

void BasicImportDialog::initOptionUi() noexcept
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
        QPushButton *restoreDefaultsButton = ui->defaultButtonBox->addButton(QDialogButtonBox::RestoreDefaults);
        connect(restoreDefaultsButton, &QPushButton::clicked,
                this, &BasicImportDialog::restoreDefaultOptions);
    } else {
        ui->optionGroupBox->setHidden(true);
    }
}

// PRIVATE SLOTS

void BasicImportDialog::onFileSelectionPushButtonClicked() noexcept
{
    // Start with the last export path
    QString exportPath = Settings::getInstance().getExportPath();

    const QString filePath = QFileDialog::getOpenFileName(this, tr("Import file..."), exportPath, d->fileFilter);
    if (!filePath.isEmpty()) {
        ui->filePathLineEdit->setText(QDir::toNativeSeparators(filePath));
    }
}

void BasicImportDialog::updateUi() noexcept
{
    const QString filePath = ui->filePathLineEdit->text();
    QFile file(filePath);
    const bool enabled = file.exists() && !ui->aircraftSelectionComboBox->currentText().isEmpty();
    d->importButton->setEnabled(enabled);
}
