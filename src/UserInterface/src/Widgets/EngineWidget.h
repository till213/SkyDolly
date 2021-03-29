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
#ifndef ENGINEWIDGET_H
#define ENGINEWIDGET_H

#include <QWidget>

#include "../../../Model/src/TimeVariableData.h"

class QShowEvent;
class QHideEvent;

class SkyConnectIntf;
class EngineData;
class EngineWidgetPrivate;

namespace Ui {
class EngineWidget;
}

class EngineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EngineWidget(SkyConnectIntf &skyConnect, QWidget *parent);
    virtual ~EngineWidget();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    Q_DISABLE_COPY(EngineWidget)
    std::unique_ptr<EngineWidgetPrivate> d;
    std::unique_ptr<Ui::EngineWidget> ui;

    void initUi();
    void updateUi(qint64 timestamp, TimeVariableData::Access access);
    const EngineData &getCurrentEngineData(qint64 timestamp, TimeVariableData::Access access) const;

private slots:
    void handleRecordedData();
    void handleTimestampChanged(qint64 timestamp, TimeVariableData::Access access);
};

#endif // ENGINEWIDGET_H
