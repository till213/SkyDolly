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
#include <memory>
#include <utility>

#include <QByteArray>

#include <Kernel/Settings.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "FormationSettings.h"

namespace
{
    // Keys
    constexpr const char *RelativePositionPlacementKey {"RelativePositionPlacement"};
    constexpr const char *FormationAircraftTableStateKey {"FormationAircraftTableState"};

    // Defaults
    constexpr bool DefaultRelativePositionPlacement {true};
}

struct FormationSettingsPrivate
{
    QByteArray formationAircraftTableState;
    bool relativePositionPlacement {::DefaultRelativePositionPlacement};
};

// PUBLIC

FormationSettings::FormationSettings() noexcept
    : ModuleBaseSettings(),
      d(std::make_unique<FormationSettingsPrivate>())
{}

FormationSettings::~FormationSettings() = default;

bool FormationSettings::isRelativePositionPlacementEnabled() const noexcept
{
    return d->relativePositionPlacement;
}

void FormationSettings::setRelativePositionPlacementEnabled(bool enable) noexcept
{
    d->relativePositionPlacement = enable;
}

QByteArray FormationSettings::getFormationAircraftTableState() const
{
    return d->formationAircraftTableState;
}

void FormationSettings::setFormationAircraftTableState(QByteArray state) noexcept
{
    d->formationAircraftTableState = std::move(state);
}

// PROTECTED

void FormationSettings::addSettingsExtn([[maybe_unused]] Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::RelativePositionPlacementKey;
    keyValue.second = d->relativePositionPlacement;
    keyValues.push_back(keyValue);

    keyValue.first = ::FormationAircraftTableStateKey;
    keyValue.second = d->formationAircraftTableState;
    keyValues.push_back(keyValue);
}

void FormationSettings::addKeysWithDefaultsExtn([[maybe_unused]] Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::RelativePositionPlacementKey;
    keyValue.second = ::DefaultRelativePositionPlacement;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::FormationAircraftTableStateKey;
    keyValue.second = QByteArray();
    keysWithDefaults.push_back(keyValue);
}

void FormationSettings::restoreSettingsExtn([[maybe_unused]] const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->relativePositionPlacement = valuesByKey.at(::RelativePositionPlacementKey).toBool();
    d->formationAircraftTableState = valuesByKey.at(::FormationAircraftTableStateKey).toByteArray();
}

void FormationSettings::restoreDefaultsExtn() noexcept
{
    d->relativePositionPlacement = ::DefaultRelativePositionPlacement;
    d->formationAircraftTableState = {};
}
