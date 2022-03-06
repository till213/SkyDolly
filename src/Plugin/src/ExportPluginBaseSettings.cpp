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
#include <memory>

#include "../../Kernel/src/Enum.h"
#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/SampleRate.h"
#include "ExportPluginBaseSettings.h"

namespace
{
    constexpr char ResamplingPeriod[] = "ResamplingPeriod";
    constexpr char OpenExportedFileEnabled[] = "OpenExportedFileEnabled";

    constexpr SampleRate::ResamplingPeriod DefaultResamplingPeriod = SampleRate::ResamplingPeriod::OneHz;
    constexpr bool DefaultOpenExportedFileEnabled = false;
}

class ExportPluginBaseSettingsPrivate
{
public:
    ExportPluginBaseSettingsPrivate()
        : resamplingPeriod(::DefaultResamplingPeriod),
          openExportedFileEnabled(::DefaultOpenExportedFileEnabled)
    {}

    SampleRate::ResamplingPeriod resamplingPeriod;
    bool openExportedFileEnabled;
};

// PUBLIC

ExportPluginBaseSettings::ExportPluginBaseSettings() noexcept
    : d(std::make_unique<ExportPluginBaseSettingsPrivate>())
{
#ifdef DEBUG
    qDebug("ExportPluginBaseSettings::ExportPluginBaseSettings: CREATED");
#endif
}

ExportPluginBaseSettings::~ExportPluginBaseSettings() noexcept
{
#ifdef DEBUG
    qDebug("ExportPluginBaseSettings::~ExportPluginBaseSettings: DELETED");
#endif
}

SampleRate::ResamplingPeriod ExportPluginBaseSettings::getResamplingPeriod() const noexcept
{
    return d->resamplingPeriod;
}

void ExportPluginBaseSettings::setResamplingPeriod(SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    if (d->resamplingPeriod != resamplingPeriod) {
        d->resamplingPeriod = resamplingPeriod;
        emit changed();
    }
}

bool ExportPluginBaseSettings::isOpenExportedFileEnabled() const noexcept
{
    return d->openExportedFileEnabled;
}

void ExportPluginBaseSettings::setOpenExportedFileEnabled(bool openExportedFileEnabled) noexcept
{
    if (d->openExportedFileEnabled != openExportedFileEnabled) {
        d->openExportedFileEnabled = openExportedFileEnabled;
        emit changed();
    }
}

void ExportPluginBaseSettings::addSettings(Settings::PluginSettings &settings) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ResamplingPeriod;
    keyValue.second = Enum::toUnderlyingType(d->resamplingPeriod);
    settings.push_back(keyValue);

    keyValue.first = ::OpenExportedFileEnabled;
    keyValue.second = d->openExportedFileEnabled;
    settings.push_back(keyValue);
}

void ExportPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ResamplingPeriod;
    keyValue.second = Enum::toUnderlyingType(DefaultResamplingPeriod);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::OpenExportedFileEnabled;
    keyValue.second = DefaultOpenExportedFileEnabled;
    keysWithDefaults.push_back(keyValue);
}

void ExportPluginBaseSettings::restoreSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    bool ok;
    int enumeration = valuesByKey[::ResamplingPeriod].toInt(&ok);
    if (ok) {
        d->resamplingPeriod = static_cast<SampleRate::ResamplingPeriod >(enumeration);
    } else {
        d->resamplingPeriod = DefaultResamplingPeriod;
    }
    d->openExportedFileEnabled = valuesByKey[::OpenExportedFileEnabled].toBool();
    emit changed();
}

void ExportPluginBaseSettings::restoreDefaults() noexcept
{
    d->resamplingPeriod = DefaultResamplingPeriod;
    d->openExportedFileEnabled = DefaultOpenExportedFileEnabled;
    emit changed();
}