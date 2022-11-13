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

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Kernel/SampleRate.h>
#include "FlightExportPluginBaseSettings.h"

namespace
{
    // Keys
    constexpr const char *ResamplingPeriodKey {"ResamplingPeriod"};
    constexpr const char *FormationExportKey {"FormationExport"};
    constexpr const char *OpenExportedFilesEnabledKey {"OpenExportedFilesEnabled"};

    // Defaults
    constexpr SampleRate::ResamplingPeriod DefaultResamplingPeriod {SampleRate::ResamplingPeriod::OneHz};
    constexpr FlightExportPluginBaseSettings::FormationExport DefaultFormationExport {FlightExportPluginBaseSettings::FormationExport::AllAircraftOneFile};
    constexpr bool DefaultOpenExportedFilesEnabled {false};
}

struct FlightExportPluginBaseSettingsPrivate
{
    FlightExportPluginBaseSettingsPrivate()
        : resamplingPeriod(::DefaultResamplingPeriod),
          formationExport(::DefaultFormationExport),
          openExportedFilesEnabled(::DefaultOpenExportedFilesEnabled),
          fileDialogSelectedFile(false)
    {}

    SampleRate::ResamplingPeriod resamplingPeriod;
    FlightExportPluginBaseSettings::FormationExport formationExport;
    bool openExportedFilesEnabled;
    bool fileDialogSelectedFile;
};

// PUBLIC

FlightExportPluginBaseSettings::FlightExportPluginBaseSettings() noexcept
    : d(std::make_unique<FlightExportPluginBaseSettingsPrivate>())
{}

FlightExportPluginBaseSettings::~FlightExportPluginBaseSettings() = default;

SampleRate::ResamplingPeriod FlightExportPluginBaseSettings::getResamplingPeriod() const noexcept
{
    return d->resamplingPeriod;
}

void FlightExportPluginBaseSettings::setResamplingPeriod(SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    if (d->resamplingPeriod != resamplingPeriod) {
        d->resamplingPeriod = resamplingPeriod;
        emit baseSettingsChanged();
    }
}

FlightExportPluginBaseSettings::FormationExport FlightExportPluginBaseSettings::getFormationExport() const noexcept
{
    return d->formationExport;
}

void FlightExportPluginBaseSettings::setFormationExport(FormationExport formationExport) noexcept
{
    if (d->formationExport != formationExport) {
        d->formationExport = formationExport;
        emit baseSettingsChanged();
    }
}

bool FlightExportPluginBaseSettings::isOpenExportedFilesEnabled() const noexcept
{
    return d->openExportedFilesEnabled;
}

void FlightExportPluginBaseSettings::setOpenExportedFilesEnabled(bool enabled) noexcept
{
    if (d->openExportedFilesEnabled != enabled) {
        d->openExportedFilesEnabled = enabled;
        emit baseSettingsChanged();
    }
}

bool FlightExportPluginBaseSettings::isFileDialogSelectedFile() const noexcept
{
    return d->fileDialogSelectedFile;
}

void FlightExportPluginBaseSettings::setFileDialogSelectedFile(bool fileDialogSelected) noexcept
{
    d->fileDialogSelectedFile = fileDialogSelected;
}

void FlightExportPluginBaseSettings::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ResamplingPeriodKey;
    keyValue.second = Enum::toUnderlyingType(d->resamplingPeriod);
    keyValues.push_back(keyValue);

    keyValue.first = ::FormationExportKey;
    keyValue.second = Enum::toUnderlyingType(d->formationExport);
    keyValues.push_back(keyValue);

    keyValue.first = ::OpenExportedFilesEnabledKey;
    keyValue.second = d->openExportedFilesEnabled;
    keyValues.push_back(keyValue);

    addSettingsExtn(keyValues);
}

void FlightExportPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ResamplingPeriodKey;
    keyValue.second = Enum::toUnderlyingType(::DefaultResamplingPeriod);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::FormationExportKey;
    keyValue.second = Enum::toUnderlyingType(::DefaultFormationExport);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::OpenExportedFilesEnabledKey;
    keyValue.second = ::DefaultOpenExportedFilesEnabled;
    keysWithDefaults.push_back(keyValue);

    addKeysWithDefaultsExtn(keysWithDefaults);
}

void FlightExportPluginBaseSettings::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    int enumeration = valuesByKey.at(::ResamplingPeriodKey).toInt(&ok);
    if (ok) {
        d->resamplingPeriod = static_cast<SampleRate::ResamplingPeriod >(enumeration);
    } else {
        d->resamplingPeriod = ::DefaultResamplingPeriod;
    }
    enumeration = valuesByKey.at(::FormationExportKey).toInt(&ok);
    if (ok) {
        d->formationExport = static_cast<FormationExport >(enumeration);
    } else {
        d->formationExport = ::DefaultFormationExport;
    }
    d->openExportedFilesEnabled = valuesByKey.at(::OpenExportedFilesEnabledKey).toBool();
    emit baseSettingsChanged();

    restoreSettingsExtn(valuesByKey);
}

void FlightExportPluginBaseSettings::restoreDefaults() noexcept
{
    d->resamplingPeriod = ::DefaultResamplingPeriod;
    d->formationExport = ::DefaultFormationExport;
    d->openExportedFilesEnabled = ::DefaultOpenExportedFilesEnabled;
    emit baseSettingsChanged();

    restoreDefaultsExtn();
}
