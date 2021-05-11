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

#include "../../../Persistence/src/Service/DatabaseService.h"
#include "../Unit.h"
#include "AboutLogbookDialog.h"
#include "ui_AboutLogbookDialog.h"

class AboutLogbookDialogPrivate
{
public:
    AboutLogbookDialogPrivate(DatabaseService &theDatabaseService) noexcept
        : databaseService(theDatabaseService)
    {}

    DatabaseService &databaseService;
};

// PUBLIC

AboutLogbookDialog::AboutLogbookDialog(DatabaseService &databaseService, QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<AboutLogbookDialogPrivate>(databaseService)),
    ui(new Ui::AboutLogbookDialog)
{
    ui->setupUi(this);
}

AboutLogbookDialog::~AboutLogbookDialog() noexcept
{
    delete ui;
}

// PROTECTED

void AboutLogbookDialog::showEvent(QShowEvent *event) noexcept
{
    Q_UNUSED(event)
    updateUi();
}

// PRIVATE

void AboutLogbookDialog::updateUi() noexcept
{
    Metadata metadata;
    d->databaseService.getMetadata(metadata);

    QString logbookPath = d->databaseService.getLogbookPath();
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
}

// PRIVATE SLOTS

void AboutLogbookDialog::on_showLogbookPathPushButton_clicked() noexcept
{
    QString logbookPath = d->databaseService.getLogbookPath();
    QFileInfo fileInfo = QFileInfo(logbookPath);
    QUrl url = QUrl::fromLocalFile(fileInfo.absolutePath());
    QDesktopServices::openUrl(url);
}
