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
#include <QLocale>

#include "../../../Persistence/src/Service/DatabaseService.h"
#include "../Unit.h"
#include "AboutLibraryDialog.h"
#include "ui_AboutLibraryDialog.h"

class AboutLibraryDialogPrivate
{
public:
    AboutLibraryDialogPrivate(DatabaseService &theDatabaseService) noexcept
        : databaseService(theDatabaseService)
    {}

    DatabaseService &databaseService;

};

// PUBLIC

AboutLibraryDialog::AboutLibraryDialog(DatabaseService &databaseService, QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<AboutLibraryDialogPrivate>(databaseService)),
    ui(new Ui::AboutLibraryDialog)
{
    ui->setupUi(this);
}

AboutLibraryDialog::~AboutLibraryDialog() noexcept
{
    delete ui;
}

// PROTECTED

void AboutLibraryDialog::showEvent(QShowEvent *event) noexcept
{
    Q_UNUSED(event)
    updateUi();
}

// PRIVATE

void AboutLibraryDialog::updateUi() noexcept
{
    Metadata metadata;
    d->databaseService.getMetadata(metadata);

    QString libraryPath = d->databaseService.getLibraryPath();
    QFileInfo fileInfo = QFileInfo(libraryPath);

    QString libraryDirectory = fileInfo.absolutePath();
    ui->directoryPathLineEdit->setText(libraryDirectory);

    QString libraryName = fileInfo.fileName();
    ui->libraryNameLineEdit->setText(libraryName);

    Unit unit;
    QLocale systemLocale = QLocale::system();
    QString createdDate = systemLocale.toString(metadata.creationDate);
    ui->createdDateLineEdit->setText(createdDate);
    QString lastOptimisationDate = systemLocale.toString(metadata.lastOptimisationDate);
    ui->lastOptimisationDateLineEdit->setText(lastOptimisationDate);
    QString lastBackupDate = systemLocale.toString(metadata.lastBackupDate);
    ui->lastBackupDateLineEdit->setText(lastBackupDate);

    qint64 fileSize = fileInfo.size();
    ui->librarySizeLineEdit->setText(unit.formatMemory(fileSize));
}
