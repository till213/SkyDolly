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
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QDesktopServices>

#include "TerminationDialog.h"
#include "ui_TerminationDialog.h"

namespace
{
    constexpr const char *OpenIssueUrl {"https://www.github.com/till213/SkyDolly/issues"};
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
}

void TerminationDialog::frenchConnection() noexcept
{
    connect(ui->createReportButton, &QPushButton::clicked,
            this, &TerminationDialog::createReport);
    connect(ui->openIssueButton, &QPushButton::clicked,
            this, &TerminationDialog::openIssue);
    connect(ui->closeButton, &QPushButton::clicked,
            this, &TerminationDialog::close);
}

// PRIVATE SLOTS

void TerminationDialog::createReport() noexcept
{

}

void TerminationDialog::openIssue() noexcept
{
    QDesktopServices::openUrl(QUrl(::OpenIssueUrl));
}
