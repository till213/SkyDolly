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
#ifndef LIGHTWIDGET_H
#define LIGHTWIDGET_H

#include <QWidget>

class QShowEvent;
class QHideEvent;

#include "../../../Model/src/TimeVariableData.h"
#include "AbstractSimulationVariableWidget.h"

class SkyConnectIntf;
struct LightData;
class LightWidgetPrivate;

namespace Ui {
    class LightWidget;
}

class LightWidget : public AbstractSimulationVariableWidget
{
    Q_OBJECT
public:
    explicit LightWidget(QWidget *parent) noexcept;
    virtual ~LightWidget() noexcept;

protected slots:
    virtual void updateUi(qint64 timestamp, TimeVariableData::Access access) noexcept override;

private:
    std::unique_ptr<LightWidgetPrivate> d;
    std::unique_ptr<Ui::LightWidget> ui;

    void initUi() noexcept;    
    const LightData &getCurrentLightData(qint64 timestamp, TimeVariableData::Access access) const noexcept;
};

#endif // LIGHTWIDGET_H
