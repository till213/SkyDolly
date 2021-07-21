/**
 * Sky Dolly - The black sheep for your flight recordings
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

#include "../../../Kernel/src/Const.h"
#include "../../../Kernel/src/Enum.h"
#include "../../../Persistence/src/Service/DatabaseService.h"
#include "../../../Persistence/src/ConnectionManager.h"
#include "../../../Persistence/src/Metadata.h"
#include "LogbookBackupDialog.h"
#include "ui_LogbookBackupDialog.h"

class LogbookBackupDialogPrivate
{
public:
    LogbookBackupDialogPrivate()
        : databaseService(std::make_unique<DatabaseService>())
    {}

    std::unique_ptr<DatabaseService> databaseService;
    QPushButton *backupButton;
    QPushButton *skipThisTimeButton;
};

// PUBLIC

LogbookBackupDialog::LogbookBackupDialog(QWidget *parent) noexcept
    : QDialog(parent),
      ui(new Ui::LogbookBackupDialog),
      d(std::make_unique<LogbookBackupDialogPrivate>())
{
    ui->setupUi(this);
    initUi();
}

LogbookBackupDialog::~LogbookBackupDialog() noexcept
{
    delete ui;
}

// PUBLIC SLOTS

void LogbookBackupDialog::accept() noexcept
{
    QDialog::accept();
    const QString path = ui->backupDirectoryLineEdit->text();
    QDir backupDir(path);
    if (backupDir.exists() || backupDir.mkpath(path)) {
        d->databaseService->backup(path);
    }
}

void LogbookBackupDialog::reject() noexcept
{
    // TODO IMPLEMENT ME!!!
   qDebug("Reject");
   QDialog::reject();
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
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    d->backupButton = new QPushButton(tr("&Backup"));
    d->backupButton->setDefault(true);

    d->skipThisTimeButton = new QPushButton(tr("&Skip This Time"));

    ui->buttonBox->addButton(d->backupButton, QDialogButtonBox::AcceptRole);
    ui->buttonBox->addButton(d->skipThisTimeButton, QDialogButtonBox::RejectRole);
}

void LogbookBackupDialog::updateUi() noexcept
{
    ConnectionManager &connectionManager = ConnectionManager::getInstance();
    Metadata metadata;
    const bool ok = connectionManager.getMetadata(metadata);
    if (ok) {
        // Backup folder
        const QString backupDirectoryPath = d->databaseService->getExistingBackupPath(metadata.backupDirectoryPath);
        ui->backupDirectoryLineEdit->setText(QDir::toNativeSeparators(backupDirectoryPath));

        // Backup period
        if (metadata.backupPeriodIntlId == Const::BackupNeverIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(Enum::toUnderlyingType(BackupPeriodComboBox::Index::Never));
        } else if (metadata.backupPeriodIntlId == Const::BackupMonthlyIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(Enum::toUnderlyingType(BackupPeriodComboBox::Index::Monthly));
        } else if (metadata.backupPeriodIntlId == Const::BackupWeeklyIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(Enum::toUnderlyingType(BackupPeriodComboBox::Index::Weekly));
        } else if (metadata.backupPeriodIntlId == Const::BackupDailyIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(Enum::toUnderlyingType(BackupPeriodComboBox::Index::Daily));
        } else if (metadata.backupPeriodIntlId == Const::BackupAlwaysIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(Enum::toUnderlyingType(BackupPeriodComboBox::Index::Always));
        } else {
            ui->backupPeriodComboBox->setCurrentIndex(Enum::toUnderlyingType(BackupPeriodComboBox::Index::Never));
        }
    }
}

// PRIVATE SLOTS

void LogbookBackupDialog::on_chooseBackupFolderPushButton_clicked() noexcept
{
    QString path = ui->backupDirectoryLineEdit->text();
    const QDir dir(path);
    if (!dir.exists()) {
        path = QFileInfo(ConnectionManager::getInstance().getLogbookPath()).absolutePath();
    }
    const QString backupDirectoryPath = QFileDialog::getExistingDirectory(this, tr("Select Backup Folder"), path);
    if (!backupDirectoryPath.isNull()) {
        ui->backupDirectoryLineEdit->setText(QDir::toNativeSeparators(backupDirectoryPath));
    }
}
