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
#ifndef MODULEINTF_H
#define MODULEINTF_H

#include <QtPlugin>
#include <QString>

class QWidget;

class ModuleIntf
{
public:
    enum struct RecordIconId
    {
        Normal,
        Add
    };

    ModuleIntf() = default;
    ModuleIntf(const ModuleIntf &rhs) = delete;
    ModuleIntf(ModuleIntf &&rhs) = default;
    ModuleIntf &operator=(const ModuleIntf &rhs) = delete;
    ModuleIntf &operator=(ModuleIntf &&rhs) = default;
    virtual ~ModuleIntf() = default;

    virtual QString getModuleName() const noexcept = 0;
    virtual QWidget *getWidget() const noexcept = 0;
    virtual RecordIconId getRecordIconId() const noexcept = 0;

    virtual void setRecording(bool enable) noexcept = 0;
    virtual void setPaused(bool enable) noexcept = 0;
    virtual void setPlaying(bool enable) noexcept = 0;

protected:
    virtual void onRecordingStopped() noexcept = 0;
};

#define MODULE_INTERFACE_IID "com.github.till213.SkyDolly.ModuleInterface/1.0"
Q_DECLARE_INTERFACE(ModuleIntf, MODULE_INTERFACE_IID)

#endif // MODULEINTF_H
