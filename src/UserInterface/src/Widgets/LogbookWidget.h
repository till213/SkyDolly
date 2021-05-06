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
#ifndef LOGBOOKWIDGET_H
#define LOGBOOKWIDGET_H

#include <memory>

#include <QWidget>

class QShowEvent;
class QHideEvent;

class SkyConnectIntf;
class LogbookWidgetPrivate;

#include "../../../Persistence/src/Service/FlightService.h"
#include "ModuleIntf.h"

namespace Ui {
    class LogbookWidget;
}

class LogbookWidget : public QWidget, public ModuleIntf
{
    Q_OBJECT

public:
    explicit LogbookWidget(FlightService &flightService, QWidget *parent = nullptr) noexcept;
    virtual ~LogbookWidget() noexcept;

    qint64 getSelectedFlightId() const noexcept;

    virtual QString getTitle() const noexcept override;

protected:
    void showEvent(QShowEvent *event) noexcept override;
    void hideEvent(QHideEvent *event) noexcept override;

private:
    Q_DISABLE_COPY(LogbookWidget)
    std::unique_ptr<Ui::LogbookWidget> ui;
    std::unique_ptr<LogbookWidgetPrivate> d;

    void initUi() noexcept;
    void updateEditUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void updateUi() noexcept;
    void handleSelectionChanged() noexcept;
    void loadFlight() noexcept;
    void deleteFlight() noexcept;
    void handleCellSelected(int row, int column) noexcept;
    void handleCellChanged(int row, int column) noexcept;
};

#endif // LOGBOOKWIDGET_H
