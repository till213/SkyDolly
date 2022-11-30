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
#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QByteArray>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QPixmap>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QTimer>

#include <Kernel/Version.h>
#include "AboutDialog.h"
#include "ui_AboutDialog.h"

struct AboutDialogPrivate
{
    AboutDialogPrivate(QWidget &parent) noexcept
    {
        if (parent.devicePixelRatioF() >= 1.5) {
            applicationPixmap.load(":/img/icons/application-icon@2x.png");
            applicationPixmap.setDevicePixelRatio(2.0);
        } else {
            applicationPixmap.load(":/img/icons/application-icon.png");
            applicationPixmap.setDevicePixelRatio(1.0);
        }
        versionInfoTimer.setSingleShot(true);
    }

    QString versionInfo;
    QTimer versionInfoTimer;
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
    updateUi();
    frenchConnection();
}

AboutDialog::~AboutDialog() = default;

// PROTECTED

void AboutDialog::mousePressEvent(QMouseEvent *event) noexcept
{
    QDialog::mousePressEvent(event);
    if (event->button() == Qt::LeftButton && !d->versionInfoTimer.isActive()) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
    }
}

void AboutDialog::mouseReleaseEvent(QMouseEvent *event) noexcept
{
    QDialog::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton && !d->versionInfoTimer.isActive()) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(d->versionInfo, QClipboard::Clipboard);
        if (clipboard->supportsSelection()) {
            clipboard->setText(d->versionInfo, QClipboard::Selection);
        }
        d->versionInfo = QStringLiteral("\n") % tr("Copied to clipboard") % QStringLiteral("\n");
        updateUi();
        d->versionInfoTimer.start(1000);
        QApplication::restoreOverrideCursor();
    }
}

// PRIVATE

void AboutDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    ui->applicationIconLabel->setPixmap(d->applicationPixmap);
    d->versionInfo = getVersionInfo();

    QFile file(":text/ThirdParty.md");
    if (file.open(QFile::ReadOnly)) {
        file.setTextModeEnabled(true);
        ui->creditsTextEdit->setMarkdown(file.readAll());
        ui->creditsTextEdit->setTextInteractionFlags(Qt::TextInteractionFlag::LinksAccessibleByMouse);
        file.close();
    }
}

void AboutDialog::updateUi() noexcept
{
    ui->aboutLabel->setText(tr("%1\nThe Black Sheep for Your Flight Recordings\n\n"
                               "%2\n\n"
                               "MIT License")
                            .arg(Version::getApplicationName(), d->versionInfo));
}

void AboutDialog::frenchConnection() noexcept
{
    connect(ui->aboutQtPushButton, &QPushButton::clicked,
            this, &AboutDialog::showAboutQtDialog);
    connect(&d->versionInfoTimer, &QTimer::timeout,
            this, &AboutDialog::restoreVersionInfo);
}

QString AboutDialog::getVersionInfo() const noexcept
{
    return QString("\"%1\" (%2)\n"
                   "Version %3 (%4)\n"
                   "%5")
          .arg(Version::getCodeName(),Version::getUserVersion(),
               Version::getApplicationVersion(), Version::getGitHash(),
               Version::getGitDate().toLocalTime().toString());
}

// PRIVATE SLOTS

void AboutDialog::showAboutQtDialog() noexcept
{
    QMessageBox::aboutQt(this);
}

void AboutDialog::restoreVersionInfo() noexcept
{
    d->versionInfo = getVersionInfo();
    updateUi();
}
