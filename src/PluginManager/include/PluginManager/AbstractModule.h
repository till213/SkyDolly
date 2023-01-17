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
#ifndef ABSTRACTMODULE_H
#define ABSTRACTMODULE_H

#include <memory>

#include <QObject>

class QWidget;

#include "SkyConnectIntf.h"
#include "ModuleIntf.h"
#include "PluginManagerLib.h"

class FlightService;
struct AbstractModulePrivate;

class PLUGINMANAGER_API AbstractModule : public QObject, public ModuleIntf
{
    Q_OBJECT
    Q_INTERFACES(ModuleIntf)
public:
    explicit AbstractModule(QObject *parent = nullptr) noexcept;
    AbstractModule(const AbstractModule &rhs) = delete;
    AbstractModule(AbstractModule &&rhs) = delete;
    AbstractModule &operator=(const AbstractModule &rhs) = delete;
    AbstractModule &operator=(AbstractModule &&rhs) = delete;
    ~AbstractModule() override;

    ModuleIntf::RecordIconId getRecordIconId() const noexcept override;
    void setRecording(bool enable) noexcept override;
    void setPauseMode(SkyConnectIntf::PauseMode pauseMode) noexcept override;
    void setPlaying(bool enable) noexcept override;

protected:
    virtual void onStartRecording() noexcept;
    virtual void onPauseMode(SkyConnectIntf::PauseMode pauseMode) noexcept;
    virtual void onStartReplay() noexcept;

    FlightService &getFlightService() const noexcept;

protected slots:
    void onRecordingStopped() noexcept override;

private:
    const std::unique_ptr<AbstractModulePrivate> d;

    void frenchConnection() noexcept;
};

#endif // ABSTRACTMODULE_H
