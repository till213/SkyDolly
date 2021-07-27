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
#ifndef PRIMARYFLIGHTCONTROLWIDGET_H
#define PRIMARYFLIGHTCONTROLWIDGET_H

#include <QWidget>

class QShowEvent;
class QHideEvent;

#include "../../../Model/src/TimeVariableData.h"
#include "AbstractSimulationVariableWidget.h"

class SkyConnectIntf;
class PrimaryFlightControlData;
class PrimaryFlightControlWidgetPrivate;

namespace Ui {
    class PrimaryFlightControlWidget;
}

class PrimaryFlightControlWidget : public AbstractSimulationVariableWidget
{
    Q_OBJECT
public:
    explicit PrimaryFlightControlWidget(QWidget *parent) noexcept;
    virtual ~PrimaryFlightControlWidget() noexcept;

protected slots:
    virtual void updateUi(qint64 timestamp, TimeVariableData::Access access) noexcept override;

private:
    std::unique_ptr<PrimaryFlightControlWidgetPrivate> d;
    std::unique_ptr<Ui::PrimaryFlightControlWidget> ui;

    void initUi();    
    const PrimaryFlightControlData &getCurrentPrimaryFlightControlData(qint64 timestamp, TimeVariableData::Access access) const noexcept;
};

#endif // PRIMARYFLIGHTCONTROLWIDGET_H
