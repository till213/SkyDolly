/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef TERMINATIONDIALOG_H
#define TERMINATIONDIALOG_H

#include <memory.h>

#include <QDialog>
#include <QTimer>

class QString;
class QTextStream;

namespace Ui {
class TerminationDialog;
}

#include "../UserInterfaceLib.h"

class USERINTERFACE_API TerminationDialog : public QDialog
{
    Q_OBJECT
public:
    TerminationDialog(QString title, QString reason, QString stackTrace, QWidget *parent = nullptr);
    virtual ~TerminationDialog();

private:
    const std::unique_ptr<Ui::TerminationDialog> ui;
    QString m_title;
    QString m_reason;
    QString m_stackTrace;

    void initUi() noexcept;
    void frenchConnection() noexcept;
    QString createReport() const noexcept;
    void enumeratePluginContent(const QString &pluginDirectoryPath, QTextStream &out) const;

private slots:
    void copyReportToClipboard() noexcept;
    void createIssue() const noexcept;
};

#endif // TERMINATIONDIALOG_H
