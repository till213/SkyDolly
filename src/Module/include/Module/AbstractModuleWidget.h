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
#ifndef ABSTRACTMODULEWIDGET_H
#define ABSTRACTMODULEWIDGET_H

#include <memory>

#include <QWidget>

class QShowEvent;
class QHideEvent;

#include "Module.h"
#include "ModuleIntf.h"

class FlightService;
class AbstractModuleWidgetPrivate;

class AbstractModuleWidget : public QWidget, public ModuleIntf
{
    Q_OBJECT
public:
    explicit AbstractModuleWidget(FlightService &flightService, QWidget *parent = nullptr) noexcept;
    virtual ~AbstractModuleWidget() noexcept;

    virtual bool isActive() const noexcept override;
    virtual void setActive(bool enable) noexcept override;

    virtual QWidget &getWidget() noexcept override;

    virtual void setRecording(bool enable) noexcept override;
    virtual void setPaused(bool enable) noexcept override;
    virtual void setPlaying(bool enable) noexcept override;

protected:
    FlightService &getFlightService() const noexcept;
    virtual void onStartRecording() noexcept;
    virtual void onPaused(bool enable) noexcept;
    virtual void onStartReplay() noexcept;

protected slots:
    virtual void handleRecordingStopped() noexcept override;

private:
    std::unique_ptr<AbstractModuleWidgetPrivate> d;
};

#endif // ABSTRACTMODULEWIDGET_H
