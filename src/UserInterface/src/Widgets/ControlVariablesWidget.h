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
#ifndef CONTROLVARIABLESWIDGET_H
#define CONTROLVARIABLESWIDGET_H

#include <QWidget>

class QShowEvent;
class QHideEvent;

class SkyConnectIntf;
class AircraftData;
class EngineData;
class ControlVariablesWidgetPrivate;

namespace Ui {
class ControlVariablesWidget;
}

class ControlVariablesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControlVariablesWidget(SkyConnectIntf &skyConnect, QWidget *parent);
    virtual ~ControlVariablesWidget();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    Q_DISABLE_COPY(ControlVariablesWidget)
    std::unique_ptr<ControlVariablesWidgetPrivate> d;
    std::unique_ptr<Ui::ControlVariablesWidget> ui;

    void initUi();
    void updateUi();
    const AircraftData &getCurrentAircraftData() const;

private slots:
    void updateControlDataUi();
};

#endif // CONTROLVARIABLESWIDGET_H
