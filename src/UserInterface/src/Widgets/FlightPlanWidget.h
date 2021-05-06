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
#ifndef FLIGHTPLANWIDGET_H
#define FLIGHTPLANWIDGET_H

#include <memory>

#include <QWidget>

class FlightPlanData;
class SkyConnectIntf;
class FlightPlanWidgetPrivate;

namespace Ui {
    class FlightPlanWidget;
}

class FlightPlanWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FlightPlanWidget(SkyConnectIntf &skyConnect, QWidget *parent = nullptr) noexcept;
    virtual ~FlightPlanWidget() noexcept;

protected:
    void showEvent(QShowEvent *event) noexcept override;
    void hideEvent(QHideEvent *event) noexcept override;

private:
    Q_DISABLE_COPY(FlightPlanWidget)
    std::unique_ptr<FlightPlanWidgetPrivate> d;
    std::unique_ptr<Ui::FlightPlanWidget> ui;

    void initUi() noexcept;
    void updateUi() noexcept;

private slots:    
    void addWaypoint(const FlightPlanData &waypoint);
    void updateWaypoint(int index, const FlightPlanData &waypoint);
    void clear();
};

#endif // FLIGHTPLANWIDGET_H
