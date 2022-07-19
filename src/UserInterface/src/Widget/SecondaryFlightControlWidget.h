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
#ifndef SECONDARYFLIGHTCONTROLWIDGET_H
#define SECONDARYFLIGHTCONTROLWIDGET_H

#include <cstdint>

#include <QWidget>

class QShowEvent;
class QHideEvent;

#include <Model/TimeVariableData.h>
#include "AbstractSimulationVariableWidget.h"

class SkyConnectIntf;
struct SecondaryFlightControlData;
class SecondaryFlightControlWidgetPrivate;

namespace Ui {
    class SecondaryFlightControlWidget;
}

class SecondaryFlightControlWidget : public AbstractSimulationVariableWidget
{
    Q_OBJECT
public:
    explicit SecondaryFlightControlWidget(QWidget *parent) noexcept;
    virtual ~SecondaryFlightControlWidget() noexcept;

protected slots:
    virtual void updateUi(std::int64_t timestamp, TimeVariableData::Access access) noexcept override;

private:
    std::unique_ptr<SecondaryFlightControlWidgetPrivate> d;
    std::unique_ptr<Ui::SecondaryFlightControlWidget> ui;

    void initUi() noexcept;    
    const SecondaryFlightControlData &getCurrentSecondaryFlightControlData(std::int64_t timestamp, TimeVariableData::Access access) const noexcept;
};

#endif // SECONDARYFLIGHTCONTROLWIDGET_H
