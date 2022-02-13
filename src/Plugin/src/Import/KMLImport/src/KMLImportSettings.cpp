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
#include "KMLImportSettings.h"

// PUBLIC

KMLImportSettings::KMLImportSettings() noexcept
{
    restoreDefaults();
}

Settings::PluginSettings KMLImportSettings::getSettings() const noexcept
{
    Settings::PluginSettings settings;
    Settings::KeyValue keyValue;

    keyValue.first = "Format";
    keyValue.second = Enum::toUnderlyingType(format);
    settings.push_back(keyValue);

    return settings;
}

Settings::KeysWithDefaults KMLImportSettings::getKeysWithDefault() const noexcept
{
    Settings::KeysWithDefaults keys;
    Settings::KeyValue keyValue;

    keyValue.first = "Format";
    keyValue.second = Enum::toUnderlyingType(KMLImportSettings::DefaultFormat);
    keys.push_back(keyValue);

    return keys;
}

void KMLImportSettings::setSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    bool ok;
    int enumeration = valuesByKey["Format"].toInt(&ok);
    if (ok) {
        format = static_cast<KMLImportSettings::Format >(enumeration);
    } else {
        format = DefaultFormat;
    }
}

void KMLImportSettings::restoreDefaults() noexcept
{
    format = DefaultFormat;
}
