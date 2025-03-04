/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef AIRCRAFTHANDLEWIDGET_H
#define AIRCRAFTHANDLEWIDGET_H

#include <cstdint>

#include <QWidget>

#include <Model/TimeVariableData.h>
#include "AbstractSimulationVariableWidget.h"

class SkyConnectIntf;
struct AircraftHandleData;
struct AircraftHandleWidgetPrivate;

namespace Ui {
    class AircraftHandleWidget;
}

class AircraftHandleWidget : public AbstractSimulationVariableWidget
{
    Q_OBJECT
public:
    explicit AircraftHandleWidget(QWidget *parent) noexcept;
    AircraftHandleWidget(const AircraftHandleWidget &rhs) = delete;
    AircraftHandleWidget(AircraftHandleWidget &&rhs) = delete;
    AircraftHandleWidget &operator=(const AircraftHandleWidget &rhs) = delete;
    AircraftHandleWidget &operator=(AircraftHandleWidget &&rhs) = delete;
    ~AircraftHandleWidget() override;

protected slots:
    void updateUi(std::int64_t timestamp, TimeVariableData::Access access) noexcept override;

private:
    const std::unique_ptr<Ui::AircraftHandleWidget> ui;
    const std::unique_ptr<AircraftHandleWidgetPrivate> d;

    void initUi() noexcept;
    AircraftHandleData getCurrentAircraftHandleData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept;
};

#endif // AIRCRAFTHANDLEWIDGET_H
