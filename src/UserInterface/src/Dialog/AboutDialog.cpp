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

#include <QDialog>
#include <QFile>
#include <QByteArray>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QPixmap>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Version.h>
#include "AboutDialog.h"
#include "ui_AboutDialog.h"

class AboutDialogPrivate
{
public:
    AboutDialogPrivate(QWidget &parent) noexcept
    {
        if (parent.devicePixelRatioF() >= 1.5) {
            applicationPixmap.load(":/img/icons/application-icon@2x.png");
            applicationPixmap.setDevicePixelRatio(2.0);
        } else {
            applicationPixmap.load(":/img/icons/application-icon.png");
            applicationPixmap.setDevicePixelRatio(1.0);
        }
    }

    QPixmap applicationPixmap;
};

// PUBLIC

AboutDialog::AboutDialog(QWidget *parent) noexcept :
    QDialog(parent),
    d(std::make_unique<AboutDialogPrivate>(*this)),
    ui(std::make_unique<Ui::AboutDialog>())
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
#ifdef DEBUG
    qDebug() << "AboutDialog::AboutDialog: CREATED";
#endif
}

AboutDialog::~AboutDialog() noexcept
{
#ifdef DEBUG
    qDebug() << "AboutDialog::~AboutDialog: DELETED";
#endif
}

// PRIVATE

void AboutDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    ui->applicationIconLabel->setPixmap(d->applicationPixmap);
    ui->aboutLabel->setText(tr("%1\nThe Black Sheep for Your Flight Recordings\n\n"
                               "Version %2\n"
                               "Git hash: %3\n"
                               "%4\n\n"
                               "MIT License")
                            .arg(Version::getApplicationName(), Version::getApplicationVersion(),
                                 Version::getGitHash(), Version::getGitDate().toLocalTime().toString()));

    QFile file(":text/ThirdParty.md");
    if (file.open(QFile::ReadOnly)) {
        file.setTextModeEnabled(true);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        ui->creditsTextEdit->setText(file.readAll().replace("\\", ""));
#else
        ui->creditsTextEdit->setMarkdown(file.readAll());
#endif
        ui->creditsTextEdit->setTextInteractionFlags(Qt::TextInteractionFlag::LinksAccessibleByMouse);
        file.close();
    }
}

void AboutDialog::frenchConnection() noexcept
{
    connect(ui->aboutQtPushButton, &QPushButton::clicked,
            this, &AboutDialog::showAboutQtDialog);
}

// PRIVATE SLOTS

void AboutDialog::showAboutQtDialog() noexcept
{
    QMessageBox::aboutQt(this);
}

