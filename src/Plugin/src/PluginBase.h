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
#ifndef PLUGINBASE_H
#define PLUGINBASE_H

#include <memory>

#include <QObject>
#include <QUuid>

#include "../../Kernel/src/Settings.h"
#include "PluginIntf.h"
#include "PluginLib.h"

class PluginBasePrivate;

class PLUGIN_API PluginBase : public QObject, public PluginIntf
{
    Q_OBJECT
public:
    PluginBase();
    virtual ~PluginBase();

    virtual QWidget *getParentWidget() const noexcept override;
    virtual void setParentWidget(QWidget *parent) noexcept override;

    virtual void storeSettings(const QUuid &pluginUuid) const noexcept override;
    virtual void restoreSettings(const QUuid &pluginUuid) noexcept override;

protected:
    virtual Settings::PluginSettings getSettings() const noexcept;
    virtual Settings::KeysWithDefaults getKeys() const noexcept;
    virtual void setSettings(Settings::ValuesByKey valuesByKey) noexcept;

private:
    std::unique_ptr<PluginBasePrivate> d;
};

#endif // PLUGINBASE_H
