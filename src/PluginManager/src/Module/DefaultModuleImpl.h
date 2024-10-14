/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef DEFAULTMODULEIMPL_H
#define DEFAULTMODULEIMPL_H

#include <memory.h>

#include <QString>
#include <QUuid>

class QWidget;

#include <Module/AbstractModule.h>

struct DefaultModuleImplPrivate;

class DefaultModuleImpl : public AbstractModule
{
	Q_OBJECT
public:
    DefaultModuleImpl() noexcept;
    DefaultModuleImpl(const DefaultModuleImpl &rhs) = delete;
    DefaultModuleImpl(DefaultModuleImpl &&rhs) = delete;
    DefaultModuleImpl &operator=(const DefaultModuleImpl &rhs) = delete;
    DefaultModuleImpl &operator=(DefaultModuleImpl &&rhs) = delete;
    ~DefaultModuleImpl() override;

    QUuid getUuid() const noexcept override;
    QString getModuleName() const noexcept final;
    QWidget *getWidget() const noexcept final;
    RecordIconId getRecordIconId() const noexcept final;

protected:
    const std::unique_ptr<DefaultModuleImplPrivate> d;

    ModuleBaseSettings &getModuleSettings() const noexcept override;
};

#endif // DEFAULTMODULEIMPL_H
