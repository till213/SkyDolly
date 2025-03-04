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

#include <memory>

#include <QString>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <PluginManager/Flight/FlightExportPluginBaseSettings.h>
#include "JsonExportSettings.h"

struct JsonExportSettingsPrivate
{};

// PUBLIC

JsonExportSettings::JsonExportSettings() noexcept
    : FlightExportPluginBaseSettings(),
      d {std::make_unique<JsonExportSettingsPrivate>()}
{}

JsonExportSettings::~JsonExportSettings() = default;

bool JsonExportSettings::isResamplingSupported() const noexcept
{
    return true;
}

bool JsonExportSettings::isFormationExportSupported(FormationExport formationExport) const noexcept
{
    bool supported {false};
    switch (formationExport) {
    case FormationExport::AllAircraftOneFile:
        supported = true;
        break;
    case FormationExport::AllAircraftSeparateFiles:
        supported = true;
        break;
    case FormationExport::UserAircraftOnly:
        supported = true;
        break;
    }
    return supported;
};

// PROTECTED

void JsonExportSettings::addSettingsExtn([[maybe_unused]] Settings::KeyValues &keyValues) const noexcept
{}

void JsonExportSettings::addKeysWithDefaultsExtn([[maybe_unused]] Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{}

void JsonExportSettings::restoreSettingsExtn([[maybe_unused]] const Settings::ValuesByKey &valuesByKey) noexcept
{}

void JsonExportSettings::restoreDefaultsExtn() noexcept
{}
