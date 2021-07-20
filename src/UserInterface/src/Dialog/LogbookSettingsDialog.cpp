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

#include <QFileInfo>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>

#include "../../../Kernel/src/Unit.h"
#include "../../../Kernel/src/Const.h"
#include "../../../Persistence/src/Service/DatabaseService.h"
#include "../../../Persistence/src/ConnectionManager.h"
#include "LogbookSettingsDialog.h"
#include "ui_LogbookSettingsDialog.h"

namespace {
    enum BackupPeriodIndex {
        Never,
        Monthly,
        Weekly,
        Daily,
        Always,
        NextTime
    };
}

class LogbookSettingsDialogPrivate
{
public:
    LogbookSettingsDialogPrivate(DatabaseService &theDatabaseService) noexcept
        : databaseService(theDatabaseService)
    {}

    DatabaseService &databaseService;
};

// PUBLIC

LogbookSettingsDialog::LogbookSettingsDialog(DatabaseService &databaseService, QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<LogbookSettingsDialogPrivate>(databaseService)),
    ui(new Ui::LogbookSettingsDialog)
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

LogbookSettingsDialog::~LogbookSettingsDialog() noexcept
{
    delete ui;
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
    ui->backupPeriodComboBox->insertItem(BackupPeriodIndex::Never, tr("Never"), Const::BackupNeverIntlId);
    ui->backupPeriodComboBox->insertItem(BackupPeriodIndex::Monthly, tr("Once a month when quitting Sky Dolly"), Const::BackupMonthlyIntlId);
    ui->backupPeriodComboBox->insertItem(BackupPeriodIndex::Weekly, tr("Once a week when quitting Sky Dolly"), Const::BackupWeeklyIntlId);
    ui->backupPeriodComboBox->insertItem(BackupPeriodIndex::Daily, tr("Daily when quitting Sky Dolly"), Const::BackupDailyIntlId);
    ui->backupPeriodComboBox->insertItem(BackupPeriodIndex::Always, tr("Always when quitting Sky Dolly"), Const::BackupAlwaysIntlId);
    ui->backupPeriodComboBox->insertItem(BackupPeriodIndex::NextTime, tr("Next time when quitting Sky Dolly"));
}

void LogbookSettingsDialog::updateUi() noexcept
{
    ConnectionManager &connectionManager = ConnectionManager::getInstance();
    Metadata metadata;
    const bool ok = connectionManager.getMetadata(metadata);
    if (ok) {
        QString logbookPath = connectionManager.getLogbookPath();
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

        qint64 fileSize = fileInfo.size();
        ui->logbookSizeLineEdit->setText(unit.formatMemory(fileSize));

        if (metadata.backupPeriodIntlId == Const::BackupNeverIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(BackupPeriodIndex::Never);
        } else if (metadata.backupPeriodIntlId == Const::BackupMonthlyIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(BackupPeriodIndex::Monthly);
        } else if (metadata.backupPeriodIntlId == Const::BackupWeeklyIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(BackupPeriodIndex::Weekly);
        } else if (metadata.backupPeriodIntlId == Const::BackupDailyIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(BackupPeriodIndex::Daily);
        } else if (metadata.backupPeriodIntlId == Const::BackupAlwaysIntlId) {
            ui->backupPeriodComboBox->setCurrentIndex(BackupPeriodIndex::Always);
        } else {
            ui->backupPeriodComboBox->setCurrentIndex(BackupPeriodIndex::Never);
        }
    }
}

void LogbookSettingsDialog::frenchConnection() noexcept
{
    connect(this, &LogbookSettingsDialog::accepted,
            this, &LogbookSettingsDialog::handleAccepted);
}

// PRIVATE SLOTS

void LogbookSettingsDialog::on_showLogbookPathPushButton_clicked() noexcept
{
    QString logbookPath = ConnectionManager::getInstance().getLogbookPath();
    QFileInfo fileInfo = QFileInfo(logbookPath);
    QUrl url = QUrl::fromLocalFile(fileInfo.absolutePath());
    QDesktopServices::openUrl(url);
}

void LogbookSettingsDialog::handleAccepted() noexcept
{
    if (ui->backupPeriodComboBox->currentIndex() != BackupPeriodIndex::NextTime) {
        const QString backupPeriodIntlId = ui->backupPeriodComboBox->currentData().toString();
        d->databaseService.updateBackupPeriod(backupPeriodIntlId);
    } else {
        // TODO IMPLEMENT ME Remember to ask user about backup upon quit
    }
}
