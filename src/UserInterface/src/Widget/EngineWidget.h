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
#ifndef ENGINEWIDGET_H
#define ENGINEWIDGET_H

#include <QWidget>

class QShowEvent;
class QHideEvent;

#include <Model/TimeVariableData.h>
#include "AbstractSimulationVariableWidget.h"

class SkyConnectIntf;
struct EngineData;
struct EngineWidgetPrivate;

namespace Ui {
    class EngineWidget;
}

class EngineWidget : public AbstractSimulationVariableWidget
{
    Q_OBJECT
public:
    explicit EngineWidget(QWidget *parent) noexcept;
    EngineWidget(const EngineWidget &rhs) = delete;
    EngineWidget(EngineWidget &&rhs) = delete;
    EngineWidget &operator=(const EngineWidget &rhs) = delete;
    EngineWidget &operator=(EngineWidget &&rhs) = delete;
    ~EngineWidget() override;

protected slots:
    void updateUi(std::int64_t timestamp, TimeVariableData::Access access) noexcept override;

private:
    const std::unique_ptr<Ui::EngineWidget> ui;
    const std::unique_ptr<EngineWidgetPrivate> d;

    void initUi() noexcept;
    EngineData getCurrentEngineData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept;
};

#endif // ENGINEWIDGET_H
