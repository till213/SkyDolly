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
#include <cstdint>

#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include <Kernel/System.h>
#include <Kernel/Settings.h>
#include <Model/Enumeration.h>
#include <Persistence/Service/EnumerationService.h>
#include <Persistence/PersistedEnumerationItem.h>
#include "CsvLocationImportSettings.h"

namespace
{
    // Keys
    constexpr const char *FormatKey {"Format"};
    constexpr const char *DefaultCountryKey {"DefaultCountry"};
    constexpr const char *DefaultAltitudeKey {"DefaultAltitude"};
    constexpr const char *DefaultIndicatedAirspeedKey {"DefaultIndicatedAirspeed"};

    // Defaults
    constexpr CsvLocationImportSettings::Format DefaultFormat {CsvLocationImportSettings::Format::SkyDolly};
}

struct CsvLocationImportSettingsPrivate
{
    CsvLocationImportSettings::Format format {::DefaultFormat};
    std::int64_t defaultCountryId {WorldCountryId};
    int defaultAltitude {Const::DefaultAltitude};
    int defaultIndicatedAirspeed {Const::DefaultIndicatedAirspeed};

    static inline std::int64_t WorldCountryId {PersistedEnumerationItem(EnumerationService::Country, EnumerationService::CountryWorldSymId).id()};
};

// PUBLIC

CsvLocationImportSettings::CsvLocationImportSettings() noexcept
    : d(std::make_unique<CsvLocationImportSettingsPrivate>())
{}

CsvLocationImportSettings::~CsvLocationImportSettings() = default;

CsvLocationImportSettings::Format CsvLocationImportSettings::getFormat() const noexcept
{
    return d->format;
}

void CsvLocationImportSettings::setFormat(Format format) noexcept
{
    if (d->format != format) {
        d->format = format;
        emit extendedSettingsChanged();
    }
}

std::int64_t CsvLocationImportSettings::getDefaultCountryId() const noexcept
{
    return d->defaultCountryId;
}

void CsvLocationImportSettings::setDefaultCountryId(std::int64_t countryId) noexcept
{
    if (d->defaultCountryId != countryId) {
        d->defaultCountryId = countryId;
        emit extendedSettingsChanged();
    }
}

int CsvLocationImportSettings::getDefaultAltitude() const noexcept
{
    return d->defaultAltitude;
}

void CsvLocationImportSettings::setDefaultAltitude(int altitude) noexcept
{
    if (d->defaultAltitude != altitude) {
        d->defaultAltitude = altitude;
        emit extendedSettingsChanged();
    }
}

int CsvLocationImportSettings::getDefaultIndicatedAirspeed() const noexcept
{
    return d->defaultIndicatedAirspeed;
}

void CsvLocationImportSettings::setDefaultIndicatedAirspeed(int indicatedAirspeed) noexcept
{
    if (d->defaultIndicatedAirspeed != indicatedAirspeed) {
        d->defaultIndicatedAirspeed = indicatedAirspeed;
        emit extendedSettingsChanged();
    }
}

// PROTECTED

void CsvLocationImportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::FormatKey;
    keyValue.second = Enum::underly(d->format);
    keyValues.push_back(keyValue);

    keyValue.first = ::DefaultCountryKey;
    keyValue.second = QVariant::fromValue(d->defaultCountryId);
    keyValues.push_back(keyValue);

    keyValue.first = ::DefaultAltitudeKey;
    keyValue.second = d->defaultAltitude;
    keyValues.push_back(keyValue);

    keyValue.first = ::DefaultIndicatedAirspeedKey;
    keyValue.second = d->defaultIndicatedAirspeed;
    keyValues.push_back(keyValue);
}

void CsvLocationImportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::FormatKey;
    keyValue.second = Enum::underly(::DefaultFormat);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::DefaultCountryKey;
    keyValue.second = QVariant::fromValue(d->WorldCountryId);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::DefaultAltitudeKey;
    keyValue.second = Const::DefaultAltitude;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::DefaultIndicatedAirspeedKey;
    keyValue.second = Const::DefaultIndicatedAirspeed;
    keysWithDefaults.push_back(keyValue);
}

void CsvLocationImportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    const int enumeration = valuesByKey.at(::FormatKey).toInt(&ok);
    if (ok) {
        d->format = static_cast<CsvLocationImportSettings::Format >(enumeration);
    } else {
        d->format = ::DefaultFormat;
    }

    const std::int64_t defaultCountryId = valuesByKey.at(::DefaultCountryKey).toLongLong(&ok);
    if (ok) {
        d->defaultCountryId = defaultCountryId;
    } else {
        d->defaultCountryId = d->WorldCountryId;
    }

    const int defaultAltitude = valuesByKey.at(::DefaultAltitudeKey).toInt(&ok);
    if (ok) {
        d->defaultAltitude = defaultAltitude;
    } else {
        d->defaultAltitude = Const::DefaultAltitude;
    }

    const int defaultIndicatedAirspeed = valuesByKey.at(::DefaultIndicatedAirspeedKey).toInt(&ok);
    if (ok) {
        d->defaultIndicatedAirspeed = defaultIndicatedAirspeed;
    } else {
        d->defaultIndicatedAirspeed = Const::DefaultIndicatedAirspeed;
    }

    emit extendedSettingsChanged();
}

void CsvLocationImportSettings::restoreDefaultsExtn() noexcept
{
    d->format = ::DefaultFormat;
    d->defaultCountryId = d->WorldCountryId;
    d->defaultAltitude = Const::DefaultAltitude;
    d->defaultIndicatedAirspeed = Const::DefaultIndicatedAirspeed;

    emit extendedSettingsChanged();
}
