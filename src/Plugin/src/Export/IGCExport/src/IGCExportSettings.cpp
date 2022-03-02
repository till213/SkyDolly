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

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/System.h"
#include "../../../../../Kernel/src/Settings.h"
#include "IGCExportSettings.h"

// PUBLIC

IGCExportSettings::IGCExportSettings() noexcept
    : defaultPilotName(System::getUsername())
{
    restoreDefaults();
}

void IGCExportSettings::addSettings(Settings::PluginSettings &settings) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = "ResamplingPeriod";
    keyValue.second = Enum::toUnderlyingType(resamplingPeriod);
    settings.push_back(keyValue);

    keyValue.first = "PilotName";
    keyValue.second = pilotName;
    settings.push_back(keyValue);

    keyValue.first = "CoPilotName";
    keyValue.second = coPilotName;
    settings.push_back(keyValue);
}

void IGCExportSettings::addKeysWithDefault(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = "ResamplingPeriod";
    keyValue.second = Enum::toUnderlyingType(IGCExportSettings::DefaultResamplingPeriod);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = "PilotName";
    keyValue.second = defaultPilotName;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = "CoPilotName";
    keyValue.second = DefaultCoPilotName;
    keysWithDefaults.push_back(keyValue);
}

void IGCExportSettings::applySettings(Settings::ValuesByKey valuesByKey) noexcept
{
    bool ok;
    int enumeration = valuesByKey["ResamplingPeriod"].toInt(&ok);
    if (ok) {
        resamplingPeriod = static_cast<IGCExportSettings::ResamplingPeriod >(enumeration);
    } else {
        resamplingPeriod = DefaultResamplingPeriod;
    }

    pilotName = valuesByKey["PilotName"].value<QString>();
    coPilotName = valuesByKey["CoPilotName"].value<QString>();
}

void IGCExportSettings::restoreDefaults() noexcept
{
    resamplingPeriod = DefaultResamplingPeriod;
    pilotName = defaultPilotName;
    coPilotName = DefaultCoPilotName;
}
