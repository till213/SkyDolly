/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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

#include <Kernel/Unit.h>
#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/Service/EnumerationService.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/PersistenceManager.h>
#include <Persistence/Metadata.h>
#include "LogbookSettingsDialog.h"
#include "ui_LogbookSettingsDialog.h"

struct LogbookSettingsDialogPrivate
{
    std::unique_ptr<DatabaseService> databaseService {std::make_unique<DatabaseService>()};
    std::int64_t originalBackupPeriodId {Const::InvalidId};

    const std::int64_t BackupPeriodNowId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodNowSymId).id()};
    const std::int64_t BackupPeriodNeverId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodNeverSymId).id()};
    const std::int64_t BackupPeriodNextTimeId {PersistedEnumerationItem(EnumerationService::BackupPeriod, EnumerationService::BackupPeriodNextTimeSymId).id()};
};

// PUBLIC

LogbookSettingsDialog::LogbookSettingsDialog(QWidget *parent) noexcept :
    QDialog {parent},
    ui {std::make_unique<Ui::LogbookSettingsDialog>()},
    d {std::make_unique<LogbookSettingsDialogPrivate>()}
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

LogbookSettingsDialog::~LogbookSettingsDialog() = default;

// PUBLIC SLOTS

void LogbookSettingsDialog::accept() noexcept
{
    QDialog::accept();
    const std::int64_t backupPeriodId = ui->backupPeriodComboBox->getCurrentId();
    if (backupPeriodId != d->originalBackupPeriodId) {
        if (ui->backupPeriodComboBox->getCurrentId() != d->BackupPeriodNextTimeId) {
            d->databaseService->setBackupPeriod(backupPeriodId);
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

    EnumerationComboBox::IgnoredIds ignoredIds;
    ignoredIds.insert(d->BackupPeriodNowId);
    ui->backupPeriodComboBox->setIgnoredIds(ignoredIds);
    ui->backupPeriodComboBox->setEnumerationName(EnumerationService::BackupPeriod);
}

void LogbookSettingsDialog::updateUi() noexcept
{
    const auto &persistenceManager = PersistenceManager::getInstance();
    bool ok {true};
    const Metadata metadata = persistenceManager.getMetadata(&ok);
    if (ok) {
        const QString logbookPath = persistenceManager.getLogbookPath();
        const QFileInfo fileInfo = QFileInfo(logbookPath);

        const QString logbookDirectoryPath = QDir::toNativeSeparators(fileInfo.absolutePath());
        ui->directoryPathLineEdit->setText(logbookDirectoryPath);

        const QString logbookName = fileInfo.fileName();
        ui->logbookNameLineEdit->setText(logbookName);

        Unit unit;
        const QString createdDate = unit.formatDateTime(metadata.creationDate);
        ui->createdDateLineEdit->setText(createdDate);
        const QString lastOptimisationDate = unit.formatDateTime(metadata.lastOptimisationDate);
        ui->lastOptimisationDateLineEdit->setText(lastOptimisationDate);
        const QString lastBackupDate = unit.formatDateTime(metadata.lastBackupDate);
        ui->lastBackupDateLineEdit->setText(lastBackupDate);

        const std::int64_t fileSize = fileInfo.size();
        ui->logbookSizeLineEdit->setText(unit.formatMemory(fileSize));
        ui->backupPeriodComboBox->setCurrentId(metadata.backupPeriodId);
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
    const QString logbookPath = PersistenceManager::getInstance().getLogbookPath();
    const QFileInfo fileInfo = QFileInfo(logbookPath);
    const QUrl url = QUrl::fromLocalFile(fileInfo.absolutePath());
    QDesktopServices::openUrl(url);
}
