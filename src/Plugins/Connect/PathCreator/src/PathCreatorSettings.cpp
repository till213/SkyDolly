/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <memory>

#include <QString>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <PluginManager/Connect/ConnectPluginBaseSettings.h>
#include "PathCreatorSettings.h"

namespace
{
    // Keys
    constexpr const char *OptionKey {"Option"};

    // Defaults
    constexpr auto DefaultOption {PathCreatorSettings::Option::A};
}

struct PathCreatorSettingsPrivate
{
    PathCreatorSettings::Option option;
};

// PUBLIC

PathCreatorSettings::PathCreatorSettings() noexcept
    : ConnectPluginBaseSettings(),
      d {std::make_unique<PathCreatorSettingsPrivate>()}
{}

PathCreatorSettings::~PathCreatorSettings() = default;

PathCreatorSettings::Option PathCreatorSettings::getOption() const noexcept
{
    return d->option;
}

void PathCreatorSettings::setOption(Option option) noexcept
{
    if (d->option != option) {
        d->option = option;
        emit changed(Connect::Mode::NotRequired);
    }
}

// PROTECTED

void PathCreatorSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::OptionKey;
    keyValue.second = Enum::underly(d->option);
    keyValues.push_back(keyValue);
}

void PathCreatorSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::OptionKey;
    keyValue.second = Enum::underly(::DefaultOption);
    keysWithDefaults.push_back(keyValue);
}

void PathCreatorSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    auto enumValue = valuesByKey.at(::OptionKey).toInt(&ok);
    d->option = ok && Enum::contains<Option>(enumValue) ? static_cast<Option>(enumValue) : ::DefaultOption;
}

void PathCreatorSettings::restoreDefaultsExtn() noexcept
{
    d->option = ::DefaultOption;
}
