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
#include <Kernel/Enum.h>
#include <Kernel/System.h>
#include <Kernel/Settings.h>
#include "CsvImportSettings.h"

namespace
{
    // Keys
    constexpr const char *FormatKey {"Format"};

    // Defaults
    constexpr CsvImportSettings::Format DefaultFormat {CsvImportSettings::Format::SkyDolly};
}

struct CsvImportSettingsPrivate
{
    CsvImportSettingsPrivate()
        : format(::DefaultFormat)
    {}

    CsvImportSettings::Format format;
};

// PUBLIC

CsvImportSettings::CsvImportSettings() noexcept
    : d(std::make_unique<CsvImportSettingsPrivate>())
{}

CsvImportSettings::~CsvImportSettings() = default;

bool CsvImportSettings::isAircraftSelectionRequired() const noexcept
{
    return true;
}

bool CsvImportSettings::isTimeOffsetSyncSupported() const noexcept
{
    bool supported {false};
    switch (d->format) {
    case CsvImportSettings::Format::FlightRadar24:
        supported = true;
        break;
    case CsvImportSettings::Format::FlightRecorder:
        supported = false;
        break;
    case CsvImportSettings::Format::SkyDolly:
        supported = false;
        break;
    }

    return supported;
}

CsvImportSettings::Format CsvImportSettings::getFormat() const noexcept
{
    return d->format;
}

void CsvImportSettings::setFormat(Format format) noexcept
{
    if (d->format != format) {
        d->format = format;
        emit changed();
    }
}

// PROTECTED

void CsvImportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::FormatKey;
    keyValue.second = Enum::underly(d->format);
    keyValues.push_back(keyValue);
}

void CsvImportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::FormatKey;
    keyValue.second = Enum::underly(::DefaultFormat);
    keysWithDefaults.push_back(keyValue);
}

void CsvImportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    const int enumeration = valuesByKey.at(::FormatKey).toInt(&ok);
    if (ok) {
        d->format = static_cast<CsvImportSettings::Format >(enumeration);
    } else {
        d->format = ::DefaultFormat;
    }
}

void CsvImportSettings::restoreDefaultsExtn() noexcept
{
    d->format = ::DefaultFormat;
}
