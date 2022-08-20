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

#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Kernel/SampleRate.h>
#include "ExportPluginBaseSettings.h"

namespace
{
    // Keys
    constexpr char ResamplingPeriodKey[] = "ResamplingPeriod";
    constexpr char FormationExportKey[] = "FormationExport";
    constexpr char OpenExportedFilesEnabledKey[] = "OpenExportedFilesEnabled";

    // Defaults
    constexpr SampleRate::ResamplingPeriod DefaultResamplingPeriod = SampleRate::ResamplingPeriod::OneHz;
    constexpr ExportPluginBaseSettings::FormationExport DefaultFormationExport = ExportPluginBaseSettings::FormationExport::AllAircraftOneFile;
    constexpr bool DefaultOpenExportedFilesEnabled = false;
}

struct ExportPluginBaseSettingsPrivate
{
    ExportPluginBaseSettingsPrivate()
        : resamplingPeriod(::DefaultResamplingPeriod),
          formationExport(::DefaultFormationExport),
          openExportedFilesEnabled(::DefaultOpenExportedFilesEnabled),
          fileDialogSelectedFile(false)
    {}

    SampleRate::ResamplingPeriod resamplingPeriod;
    ExportPluginBaseSettings::FormationExport formationExport;
    bool openExportedFilesEnabled;
    bool fileDialogSelectedFile;
};

// PUBLIC

ExportPluginBaseSettings::ExportPluginBaseSettings() noexcept
    : d(std::make_unique<ExportPluginBaseSettingsPrivate>())
{
#ifdef DEBUG
    qDebug() << "ExportPluginBaseSettings::ExportPluginBaseSettings: CREATED";
#endif
}

ExportPluginBaseSettings::~ExportPluginBaseSettings() noexcept
{
#ifdef DEBUG
    qDebug() << "ExportPluginBaseSettings::~ExportPluginBaseSettings: DELETED";
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
        emit baseSettingsChanged();
    }
}

ExportPluginBaseSettings::FormationExport ExportPluginBaseSettings::getFormationExport() const noexcept
{
    return d->formationExport;
}

void ExportPluginBaseSettings::setFormationExport(FormationExport formationExport) noexcept
{
    if (d->formationExport != formationExport) {
        d->formationExport = formationExport;
        emit baseSettingsChanged();
    }
}

bool ExportPluginBaseSettings::isOpenExportedFilesEnabled() const noexcept
{
    return d->openExportedFilesEnabled;
}

void ExportPluginBaseSettings::setOpenExportedFilesEnabled(bool enabled) noexcept
{
    if (d->openExportedFilesEnabled != enabled) {
        d->openExportedFilesEnabled = enabled;
        emit baseSettingsChanged();
    }
}

bool ExportPluginBaseSettings::isFileDialogSelectedFile() const noexcept
{
    return d->fileDialogSelectedFile;
}

void ExportPluginBaseSettings::setFileDialogSelectedFile(bool fileDialogSelected) noexcept
{
    d->fileDialogSelectedFile = fileDialogSelected;
}

void ExportPluginBaseSettings::addSettings(Settings::KeyValues &keyValues) const noexcept
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

void ExportPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
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

void ExportPluginBaseSettings::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok;
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

void ExportPluginBaseSettings::restoreDefaults() noexcept
{
    d->resamplingPeriod = ::DefaultResamplingPeriod;
    d->formationExport = ::DefaultFormationExport;
    d->openExportedFilesEnabled = ::DefaultOpenExportedFilesEnabled;
    emit baseSettingsChanged();

    restoreDefaultsExtn();
}
