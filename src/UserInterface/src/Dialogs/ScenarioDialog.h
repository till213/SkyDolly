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
#ifndef SCENARIODIALOG_H
#define SCENARIODIALOG_H

#include <memory>

#include <QDialog>

class QShowEvent;
class QHideEvent;

class SkyConnectIntf;
class ScenarioDialogPrivate;

namespace Ui {
class ScenarioDialog;
}

class ScenarioDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScenarioDialog(SkyConnectIntf &skyConnect, QWidget *parent = nullptr) noexcept;
    virtual ~ScenarioDialog() noexcept;

signals:
    void visibilityChanged(bool visible);

protected:
    void showEvent(QShowEvent *event) noexcept override;
    void hideEvent(QHideEvent *event) noexcept override;

private:
    Q_DISABLE_COPY(ScenarioDialog)
    std::unique_ptr<ScenarioDialogPrivate> d;
    std::unique_ptr<Ui::ScenarioDialog> ui;

    void initUi() noexcept;
    void updateUi() noexcept;
};

#endif // SCENARIODIALOG_H
