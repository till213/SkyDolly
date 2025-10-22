/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <PluginManager/Flight/FlightExportPluginBaseSettings.h>
#include "CsvExportSettings.h"

namespace
{
    // Keys
    constexpr const char *FormatKey {"Format"};

    // Defaults
    constexpr CsvExportSettings::Format DefaultFormat {CsvExportSettings::Format::PositionAndAttitude};
}

struct CsvExportSettingsPrivate
{
    CsvExportSettings::Format format {::DefaultFormat};
};

// PUBLIC

CsvExportSettings::CsvExportSettings() noexcept
    : FlightExportPluginBaseSettings(),
      d {std::make_unique<CsvExportSettingsPrivate>()}
{}

CsvExportSettings::~CsvExportSettings() = default;

CsvExportSettings::Format CsvExportSettings::getFormat() const noexcept
{
    return d->format;
}

void CsvExportSettings::setFormat(Format format) noexcept
{
    if (d->format != format) {
        d->format = format;
        emit changed();
    }
}

bool CsvExportSettings::isResamplingSupported() const noexcept
{
    return true;
}

bool CsvExportSettings::isFormationExportSupported(FormationExport formationExport) const noexcept
{
    bool supported {false};
    switch (formationExport) {
    case FormationExport::AllAircraftOneFile:
        supported = false;
        break;
    case FormationExport::AllAircraftSeparateFiles:
        supported = false;
        break;
    case FormationExport::UserAircraftOnly:
        supported = true;
        break;
    }
    return supported;
};

// PROTECTED

void CsvExportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::FormatKey;
    keyValue.second = Enum::underly(d->format);
    keyValues.push_back(keyValue);
}

void CsvExportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::FormatKey;
    keyValue.second = Enum::underly(::DefaultFormat);
    keysWithDefaults.push_back(keyValue);
}

void CsvExportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    auto enumValue = valuesByKey.at(::FormatKey).toInt(&ok);
    d->format = ok && Enum::contains<Format>(enumValue) ? static_cast<Format>(enumValue) : ::DefaultFormat;
}

void CsvExportSettings::restoreDefaultsExtn() noexcept
{
    d->format = ::DefaultFormat;
}
