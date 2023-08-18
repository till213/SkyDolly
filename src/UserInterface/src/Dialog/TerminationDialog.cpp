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
#include <utility>

#include <QUrl>
#include <QWidget>
#include <QString>
#include <QTextStream>
#include <QStringBuilder>
#include <QIODeviceBase>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QDesktopServices>
#include <QLabel>
#include <QStyle>
#include <QFontDatabase>
#include <QClipboard>
#include <QTimer>
#include <QSysInfo>

#include <Kernel/Version.h>
#include <Kernel/Settings.h>
#include "Dialog/TerminationDialog.h"
#include "ui_TerminationDialog.h"

namespace
{
    constexpr int InfoTimerInterval {5000};
    constexpr const char *CreateIssueUrl {"https://www.github.com/till213/SkyDolly/issues/new"};
    constexpr const char *CreateIssueTemplateUrl {"https://github.com/till213/SkyDolly/issues/new?template=bug_report.md&title=Unexpected%20Termination"};
}

// PUBLIC

TerminationDialog::TerminationDialog(
    const QString title,
    const QString exception,
    const QString stackTrace,
    QWidget *parent
 ) : QDialog(parent),
    ui(new Ui::TerminationDialog),
    m_title(std::move(title)),
    m_exception(std::move(exception)),
    m_stackTrace(std::move(stackTrace))
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

TerminationDialog::~TerminationDialog()
{
    delete ui;
}

// PRIVATE

void TerminationDialog::initUi() noexcept
{
    this->setWindowTitle(m_title);
    ui->exceptionTextEdit->appendPlainText(m_exception);
    ui->stackTraceTextEdit->appendPlainText(m_stackTrace);

    ui->iconLabel->setPixmap(this->style()->standardPixmap(QStyle::SP_MessageBoxCritical));
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->stackTraceTextEdit->setFont(fixedFont);
    QString explanation {
"<html><head/><body>"
"<p>" % tr("The application quit unexpectedly: this is due to a programming error. Sky Dolly deeply apologises.") % "</p>"
"<p>" % tr("You can help fixing this bug by creating a new issue at") % " " % "<a href=\"" % ::CreateIssueTemplateUrl % "\">"
"<span style=\"text-decoration: underline; color:#007af4;\">" % ::CreateIssueUrl % "</span></a> " %
tr("(free github.com account required).") % "</p></body></html>"
    };
    ui->explanationLabel->setText(explanation);
}

void TerminationDialog::frenchConnection() noexcept
{
    connect(ui->copyReportButton, &QPushButton::clicked,
            this, &TerminationDialog::copyReportToClipboard);
    connect(ui->createIssueButton, &QPushButton::clicked,
            this, &TerminationDialog::createIssue);
    connect(ui->closeButton, &QPushButton::clicked,
            this, &TerminationDialog::close);
}

QString TerminationDialog::createReport() const noexcept
{
    QString report;
    QTextStream out(&report, QIODeviceBase::WriteOnly);

    Settings &settings = Settings::getInstance();
    out << this->windowTitle() << Qt::endl << Qt::endl
        << "Exception:" << Qt::endl << Qt::endl
        << ui->exceptionTextEdit->toPlainText() << Qt::endl << Qt::endl
        << ui->stackTraceTextEdit->toPlainText() << Qt::endl << Qt::endl
        << "Application:" << Qt::endl
        << "Sky Dolly version: " << Version::getApplicationVersion() << Qt::endl
        << "Logboog path: " << settings.getLogbookPath() << Qt::endl << Qt::endl
        << "System info: " << Qt::endl
        << "Kernel version: " << QSysInfo::kernelVersion() << Qt::endl
        << "Kernel type: " << QSysInfo::kernelType() << Qt::endl
        << "Product name:  " << QSysInfo::prettyProductName() << Qt::endl
        << "Product type:  " << QSysInfo::productType() << Qt::endl
        << "Product version:  " << QSysInfo::productVersion() << Qt::endl;

    return report;
}

// PRIVATE SLOTS

void TerminationDialog::copyReportToClipboard()  noexcept
{
    QString report = createReport();
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(report, QClipboard::Clipboard);
    if (clipboard->supportsSelection()) {
        clipboard->setText(report, QClipboard::Selection);
    }
    ui->infoLabel->setText(tr("Report copied to clipboard. You may now paste it in your browser into the created issue."));
}

void TerminationDialog::createIssue() const noexcept
{
    QDesktopServices::openUrl(QUrl(::CreateIssueUrl));
    ui->infoLabel->setText(tr("You may now create a report by clicking on the Copy Report to Clipboard button."));
}
