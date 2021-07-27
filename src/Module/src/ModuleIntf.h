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
#ifndef MODULEINTF_H
#define MODULEINTF_H

#include <QString>

class QWidget;
class QAction;

#include "Module.h"

class ModuleIntf
{
public:
    virtual ~ModuleIntf() {};
    virtual Module::Module getModuleId() const noexcept = 0;
    virtual const QString getModuleName() const noexcept = 0;

    virtual bool isActive() const noexcept = 0;
    virtual void setActive(bool enable) noexcept = 0;

    virtual QWidget &getWidget() noexcept = 0;
    virtual QAction &getAction() noexcept = 0;

    virtual void setRecording(bool enable) noexcept = 0;
    virtual void setPaused(bool enable) noexcept = 0;
    virtual void setPlaying(bool enable) noexcept = 0;

protected:
    virtual void handleRecordingStopped() noexcept = 0;
};

#endif // MODULEINTF_H
