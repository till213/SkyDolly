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
#include "CSVImportSettings.h"

namespace
{
    constexpr char FormatKey[] = "Format";

    constexpr CSVImportSettings::Format DefaultFormat = CSVImportSettings::Format::SkyDolly;
}

class CSVImportSettingsPrivate
{
public:
    CSVImportSettingsPrivate()
        : format(::DefaultFormat)
    {}

    CSVImportSettings::Format format;
};

// PUBLIC

CSVImportSettings::CSVImportSettings() noexcept
    : d(std::make_unique<CSVImportSettingsPrivate>())
{
#ifdef DEBUG
    qDebug("CSVImportSettings::CSVImportSettings: CREATED");
#endif
}

CSVImportSettings::~CSVImportSettings() noexcept
{
#ifdef DEBUG
    qDebug("CSVImportSettings::~CSVImportSettings: DELETED");
#endif
}

CSVImportSettings::Format CSVImportSettings::getFormat() const noexcept
{
    return d->format;
}

void CSVImportSettings::setFormat(Format format) noexcept
{
    if (d->format != format) {
        d->format = format;
        emit extendedSettingsChanged();
    }
}

// PROTECTED

void CSVImportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::FormatKey;
    keyValue.second = Enum::toUnderlyingType(d->format);
    keyValues.push_back(keyValue);
}

void CSVImportSettings::setTimestampMode(TimestampMode timestampMode) noexcept
{
    if (d->timestampMode != timestampMode) {
        d->timestampMode = timestampMode;
        emit extendedSettingsChanged();
    }
}


void CSVImportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::FormatKey;
    keyValue.second = Enum::toUnderlyingType(::DefaultFormat);
    keysWithDefaults.push_back(keyValue);
}

void CSVImportSettings::applySettingsExtn(Settings::ValuesByKey valuesByKey) noexcept
{
    bool ok;
    const int enumeration = valuesByKey[::FormatKey].toInt(&ok);
    if (ok) {
        d->format = static_cast<CSVImportSettings::Format >(enumeration);
    } else {
        d->format = ::DefaultFormat;
    }
}

void CSVImportSettings::restoreDefaults() noexcept
{
    d->format = ::DefaultFormat;

    emit extendedSettingsChanged();
}
