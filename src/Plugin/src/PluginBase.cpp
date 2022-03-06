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
#include <QWidget>
#include <QUuid>

#include "../../Kernel/src/Settings.h"
#include "PluginBase.h"

class PluginBasePrivate
{
public:
    PluginBasePrivate()
        : parent(nullptr)
    {}

    QWidget *parent;
};

// PUBLIC

PluginBase::PluginBase()
    : d(std::make_unique<PluginBasePrivate>())
{}

PluginBase::~PluginBase()
{}

QWidget *PluginBase::getParentWidget() const noexcept
{
    return d->parent;
}

void PluginBase::setParentWidget(QWidget *parent) noexcept
{
    d->parent = parent;
}

void PluginBase::storeSettings(const QUuid &pluginUuid) const noexcept
{
    Settings::PluginSettings settings;
    addSettings(settings);
    if (settings.size() > 0) {
        Settings::getInstance().storePluginSettings(pluginUuid, settings);
    }
}

void PluginBase::restoreSettings(const QUuid &pluginUuid) noexcept
{
    Settings::KeysWithDefaults keysWithDefaults;
    addKeysWithDefaults(keysWithDefaults);
    if (keysWithDefaults.size() > 0) {
        Settings::ValuesByKey values = Settings::getInstance().restorePluginSettings(pluginUuid, keysWithDefaults);
        restoreSettings(values);
    }
}

// PROTECTED

void PluginBase::addSettings(Settings::PluginSettings &settings) const noexcept
{}

void PluginBase::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{}

void PluginBase::restoreSettings([[maybe_unused]]Settings::ValuesByKey valuesByKey) noexcept
{}
