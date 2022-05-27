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
#include <memory>
#include <cstdint>

#include <QFileInfo>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Unit.h>
#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/LogbookManager.h>
#include <Persistence/Metadata.h>
#include <Widget/BackupPeriodComboBox.h>
#include "LogbookSettingsDialog.h"
#include "ui_LogbookSettingsDialog.h"

class LogbookSettingsDialogPrivate
{
public:
    LogbookSettingsDialogPrivate() noexcept
        : databaseService(std::make_unique<DatabaseService>())
    {}

    std::unique_ptr<DatabaseService> databaseService;
    QString originalBackupPeriodIntlId;
};

// PUBLIC

LogbookSettingsDialog::LogbookSettingsDialog(QWidget *parent) noexcept :
    QDialog(parent),
    ui(std::make_unique<Ui::LogbookSettingsDialog>()),
    d(std::make_unique<LogbookSettingsDialogPrivate>())
{
    ui->setupUi(this);
    initUi();
    frenchConnection();

    LogbookManager &logbookManager = LogbookManager::getInstance();
    Metadata metadata;
    if (logbookManager.getMetadata(metadata)) {
        d->originalBackupPeriodIntlId = metadata.backupPeriodIntlId;
    } else {
        d->originalBackupPeriodIntlId = Const::BackupNeverIntlId;
    }
#ifdef DEBUG
    qDebug() << "LogbookSettingsDialog::LogbookSettingsDialog: CREATED";
#endif
}

LogbookSettingsDialog::~LogbookSettingsDialog() noexcept
{
#ifdef DEBUG
    qDebug() << "LogbookSettingsDialog::~LogbookSettingsDialog: DELETED";
#endif
}

// PUBLIC SLOTS

void LogbookSettingsDialog::accept() noexcept
{
    QDialog::accept();
    const QString backupPeriodIntlId = ui->backupPeriodComboBox->currentData().toString();
    if (backupPeriodIntlId != d->originalBackupPeriodIntlId) {
        if (ui->backupPeriodComboBox->currentIndex() != Enum::toUnderlyingType(BackupPeriodComboBox::Index::NextTime)) {
            d->databaseService->setBackupPeriod(backupPeriodIntlId);
            d->databaseService->updateBackupDate();
        } else {
            // Ask next time Sky Dolly is quitting
            d->databaseService->setNextBackupDate(QDateTime::currentDateTime());
        }
    }
    Settings::getInstance().setBackupBeforeMigrationEnabled(ui->backupBeforeMigrationCheckBox->isChecked());
}

// PROTECTED

void LogbookSettingsDialog::showEvent(QShowEvent *event) noexcept
{
    QDialog::showEvent(event);
    updateUi();
}

// PRIVATE

void LogbookSettingsDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    ui->backupPeriodComboBox->setSelection(BackupPeriodComboBox::Selection::IncludingNextTime);
}

void LogbookSettingsDialog::updateUi() noexcept
{
    LogbookManager &logbookManager = LogbookManager::getInstance();
    Metadata metadata;
    const bool ok = logbookManager.getMetadata(metadata);
    if (ok) {
        QString logbookPath = logbookManager.getLogbookPath();
        QFileInfo fileInfo = QFileInfo(logbookPath);

        QString logbookDirectoryPath = QDir::toNativeSeparators(fileInfo.absolutePath());
        ui->directoryPathLineEdit->setText(logbookDirectoryPath);

        QString logbookName = fileInfo.fileName();
        ui->logbookNameLineEdit->setText(logbookName);

        Unit unit;
        QString createdDate = unit.formatDateTime(metadata.creationDate);
        ui->createdDateLineEdit->setText(createdDate);
        QString lastOptimisationDate = unit.formatDateTime(metadata.lastOptimisationDate);
        ui->lastOptimisationDateLineEdit->setText(lastOptimisationDate);
        QString lastBackupDate = unit.formatDateTime(metadata.lastBackupDate);
        ui->lastBackupDateLineEdit->setText(lastBackupDate);

        std::int64_t fileSize = fileInfo.size();
        ui->logbookSizeLineEdit->setText(unit.formatMemory(fileSize));

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
    ui->backupBeforeMigrationCheckBox->setChecked(Settings::getInstance().isBackupBeforeMigrationEnabled());
}

void LogbookSettingsDialog::frenchConnection() noexcept
{
    connect(ui->showLogbookPathPushButton, &QPushButton::clicked,
            this, &LogbookSettingsDialog::openLogbookDirectory);
}

// PRIVATE SLOTS

void LogbookSettingsDialog::openLogbookDirectory() noexcept
{
    const QString logbookPath = LogbookManager::getInstance().getLogbookPath();
    const QFileInfo fileInfo = QFileInfo(logbookPath);
    const QUrl url = QUrl::fromLocalFile(fileInfo.absolutePath());
    QDesktopServices::openUrl(url);
}
