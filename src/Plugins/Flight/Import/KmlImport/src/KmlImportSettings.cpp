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
#include <QString>

#include <Kernel/Enum.h>
#include <Kernel/System.h>
#include <Kernel/Settings.h>
#include "KmlImportSettings.h"

namespace
{
    // Keys
    constexpr const char *FormatKey {"Format"};

    // Defaults
    constexpr KmlImportSettings::Format DefaultFormat {KmlImportSettings::Format::FlightAware};
}

struct KmlImportSettingsPrivate
{
    KmlImportSettingsPrivate()
        : format(::DefaultFormat)
    {}

    KmlImportSettings::Format format;
};

// PUBLIC

KmlImportSettings::KmlImportSettings() noexcept
    : d {std::make_unique<KmlImportSettingsPrivate>()}
{}

KmlImportSettings::~KmlImportSettings() = default;

bool KmlImportSettings::isAircraftSelectionRequired() const noexcept
{
    return true;
}

bool KmlImportSettings::isTimeOffsetSyncSupported() const noexcept
{
    return true;
}

KmlImportSettings::Format KmlImportSettings::getFormat() const noexcept
{
    return d->format;
}

void KmlImportSettings::setFormat(Format format) noexcept
{
    if (d->format != format) {
        d->format = format;
        emit changed();
    }
}

// PROTECTED

void KmlImportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = QString::fromLatin1(::FormatKey);
    keyValue.second = Enum::underly(d->format);
    keyValues.push_back(keyValue);
}

void KmlImportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = QString::fromLatin1(::FormatKey);
    keyValue.second = Enum::underly(::DefaultFormat);
    keysWithDefaults.push_back(keyValue);
}

void KmlImportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    auto enumValue = valuesByKey.at(QString::fromLatin1(::FormatKey)).toInt(&ok);
    d->format = ok && Enum::contains<Format>(enumValue) ? static_cast<Format>(enumValue) : ::DefaultFormat;
}

void KmlImportSettings::restoreDefaultsExtn() noexcept
{
    d->format = ::DefaultFormat;
}
