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
#ifndef MODULEPLUGINBASE_H
#define MODULEPLUGINBASE_H

#include <memory>

#include <QObject>

class QWidget;

#include "Module.h"
#include "ModuleIntf.h"
#include "PluginManagerLib.h"

class FlightService;
struct ModulePluginBasePrivate;

class PLUGINMANAGER_API ModulePluginBase : public QObject, public ModuleIntf
{
    Q_OBJECT
public:
    explicit ModulePluginBase(QObject *parent = nullptr) noexcept;
    ~ModulePluginBase() noexcept override;

    bool isActive() const noexcept override;
    void setActive(bool enable) noexcept override;

    void setRecording(bool enable) noexcept override;
    void setPaused(bool enable) noexcept override;
    void setPlaying(bool enable) noexcept override;

protected:
    virtual void onStartRecording() noexcept;
    virtual void onPaused(bool enable) noexcept;
    virtual void onStartReplay() noexcept;

protected slots:
    void onRecordingStopped() noexcept override;

private:
    std::unique_ptr<ModulePluginBasePrivate> d;
};

#endif // MODULEPLUGINBASE_H
