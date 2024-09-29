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
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/PersistenceManager.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/Service/EnumerationService.h>
#include <Persistence/Metadata.h>
#include <Widget/EnumerationComboBox.h>
#include "LogbookBackupDialog.h"
#include "ui_LogbookBackupDialog.h"

struct LogbookBackupDialogPrivate
{
    std::unique_ptr<DatabaseService> databaseService {std::make_unique<DatabaseService>()};
    std::int64_t originalBackupPeriodId {Const::InvalidId};

    const std::int64_t BackupPeriodNeverId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodNeverSymId).id()};
    const std::int64_t BackupPeriodNowId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodNowSymId).id()};
    const std::int64_t BackupPeriodNextTimeId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodNextTimeSymId).id()};
};

// PUBLIC

LogbookBackupDialog::LogbookBackupDialog(QWidget *parent) noexcept
    : QDialog {parent},
      ui {std::make_unique<Ui::LogbookBackupDialog>()},
      d {std::make_unique<LogbookBackupDialogPrivate>()}
{
    ui->setupUi(this);
    initUi();
    frenchConnection();

    const auto &persistenceManager = PersistenceManager::getInstance();
    bool ok {true};
    const Metadata metadata = persistenceManager.getMetadata(&ok);
    if (ok) {
        d->originalBackupPeriodId = metadata.backupPeriodId;
    } else {
        d->originalBackupPeriodId = d->BackupPeriodNeverId;
    }
}

LogbookBackupDialog::~LogbookBackupDialog() = default;

// PUBLIC SLOTS

void LogbookBackupDialog::accept() noexcept
{
    QDialog::accept();

    // Update the backup directory
    bool ok = d->databaseService->setBackupDirectoryPath(ui->backupDirectoryLineEdit->text());

    // First update the backup period, as this influences...
    if (ok) {
        const auto backupPeriodId = ui->backupPeriodComboBox->getCurrentId();
        if (backupPeriodId != d->BackupPeriodNowId) {
            ok = d->databaseService->setBackupPeriod(backupPeriodId);
        } else {
            // Only do the backup now, but afterwards never
           ok = d->databaseService->setBackupPeriod(d->BackupPeriodNeverId);
        }
    }

    // ... the next backup date which is set upon successful backup
    if (ok) {
        const auto &persistenceManager = PersistenceManager::getInstance();
        ok =d->databaseService->backup(persistenceManager.getLogbookPath(), DatabaseService::BackupMode::Normal);
    }

    if (!ok) {
        QMessageBox::critical(this, tr("Backup Error"), tr("The logbook backup could not be created."));
    }
}

void LogbookBackupDialog::reject() noexcept
{
   QDialog::reject();

   // First update the backup period in case it has been changed...
   const auto backupPeriodId = ui->backupPeriodComboBox->getCurrentId();
   if (backupPeriodId != d->originalBackupPeriodId) {
       // ... as this influences...
       if (backupPeriodId != d->BackupPeriodNowId) {
           d->databaseService->setBackupPeriod(backupPeriodId);
       } else {
           // Skip this backup, and also afterwards
           d->databaseService->setBackupPeriod(d->BackupPeriodNeverId);
       }
       // ... the next backup date
       d->databaseService->updateBackupDate();
   }
}

// PROTECTED

void LogbookBackupDialog::showEvent(QShowEvent *event) noexcept
{
    QDialog::showEvent(event);
    updateUi();
}

// PRIVATE

void LogbookBackupDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    EnumerationComboBox::IgnoredIds ignoredIds;
    ignoredIds.insert(d->BackupPeriodNeverId);
    ignoredIds.insert(d->BackupPeriodNextTimeId);
    ui->backupPeriodComboBox->setIgnoredIds(ignoredIds);
    ui->backupPeriodComboBox->setEnumerationName(EnumerationService::BackupPeriod);

    // Transfer ownership to the buttonBox
    QPushButton *backupButton = ui->buttonBox->addButton(tr("&Backup"), QDialogButtonBox::AcceptRole);
    backupButton->setDefault(true);
    ui->buttonBox->addButton(tr("&Skip This Time"), QDialogButtonBox::RejectRole);
}

void LogbookBackupDialog::updateUi() noexcept
{
    const auto &persistenceManager = PersistenceManager::getInstance();
    bool ok {true};
    const Metadata metadata = persistenceManager.getMetadata(&ok);
    if (ok) {
        // Backup folder
        ui->backupDirectoryLineEdit->setText(QDir::toNativeSeparators(metadata.backupDirectoryPath));
        ui->backupPeriodComboBox->setCurrentId(metadata.backupPeriodId);
    }
}

void LogbookBackupDialog::frenchConnection() noexcept
{
    connect(ui->chooseBackupFolderPushButton, &QPushButton::clicked,
            this, &LogbookBackupDialog::chooseBackupFolder);
}

// PRIVATE SLOTS

void LogbookBackupDialog::chooseBackupFolder() noexcept
{
    QString path = ui->backupDirectoryLineEdit->text();
    const QDir dir(path);
    if (!dir.exists()) {
        path = QFileInfo(PersistenceManager::getInstance().getLogbookPath()).absolutePath();
    }
    const QString backupDirectoryPath = QFileDialog::getExistingDirectory(this, tr("Select Backup Folder"), path);
    if (!backupDirectoryPath.isNull()) {
        ui->backupDirectoryLineEdit->setText(QDir::toNativeSeparators(backupDirectoryPath));
    }
}
