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
#include "../../Kernel/src/Enum.h"
#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/SampleRate.h"
#include "ExportPluginBaseSettings.h"

namespace
{
    constexpr char ResamplingPeriod[] = "ResamplingPeriod";
    constexpr char OpenExportedFile[] = "OpenExportedFile";
}

// PUBLIC

ExportPluginBaseSettings::ExportPluginBaseSettings() noexcept
{
    initSettings();
}

ExportPluginBaseSettings::~ExportPluginBaseSettings() noexcept
{}

void ExportPluginBaseSettings::addSettings(Settings::PluginSettings &settings) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ResamplingPeriod;
    keyValue.second = Enum::toUnderlyingType(resamplingPeriod);
    settings.push_back(keyValue);

    keyValue.first = ::OpenExportedFile;
    keyValue.second = openExportedFile;
    settings.push_back(keyValue);
}

void ExportPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ResamplingPeriod;
    keyValue.second = Enum::toUnderlyingType(DefaultResamplingPeriod);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::OpenExportedFile;
    keyValue.second = DefaultOpenExportedFile;
    keysWithDefaults.push_back(keyValue);
}

void ExportPluginBaseSettings::applySettings(Settings::ValuesByKey valuesByKey) noexcept
{
    bool ok;
    int enumeration = valuesByKey[::ResamplingPeriod].toInt(&ok);
    if (ok) {
        resamplingPeriod = static_cast<SampleRate::ResamplingPeriod >(enumeration);
    } else {
        resamplingPeriod = DefaultResamplingPeriod;
    }
    openExportedFile = valuesByKey[::OpenExportedFile].toBool();
}

void ExportPluginBaseSettings::restoreDefaults() noexcept
{
    initSettings();
    emit defaultsRestored();
}

// PRIVATE

void ExportPluginBaseSettings::initSettings() noexcept
{
    resamplingPeriod = DefaultResamplingPeriod;
    openExportedFile = DefaultOpenExportedFile;
}
