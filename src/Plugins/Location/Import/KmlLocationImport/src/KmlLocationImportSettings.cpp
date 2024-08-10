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
#include "KmlLocationImportSettings.h"

namespace
{
    // Keys
    constexpr const char *DefaultCountryKey {"DefaultCountry"};
    constexpr const char *DefaultAltitudeKey {"DefaultAltitude"};
    constexpr const char *DefaultIndicatedAirspeedKey {"DefaultIndicatedAirspeed"};
}

struct KmlLocationImportSettingsPrivate
{
    std::int64_t defaultCountryId {WorldCountryId};
    int defaultAltitude {Const::DefaultAltitude};
    int defaultIndicatedAirspeed {Const::DefaultIndicatedAirspeed};

    static inline const std::int64_t WorldCountryId {PersistedEnumerationItem(EnumerationService::Country, EnumerationService::CountryWorldSymId).id()};
};

// PUBLIC

KmlLocationImportSettings::KmlLocationImportSettings() noexcept
    : d {std::make_unique<KmlLocationImportSettingsPrivate>()}
{}

KmlLocationImportSettings::~KmlLocationImportSettings() = default;

std::int64_t KmlLocationImportSettings::getDefaultCountryId() const noexcept
{
    return d->defaultCountryId;
}

void KmlLocationImportSettings::setDefaultCountryId(std::int64_t countryId) noexcept
{
    if (d->defaultCountryId != countryId) {
        d->defaultCountryId = countryId;
        emit changed();
    }
}

int KmlLocationImportSettings::getDefaultAltitude() const noexcept
{
    return d->defaultAltitude;
}

void KmlLocationImportSettings::setDefaultAltitude(int altitude) noexcept
{
    if (d->defaultAltitude != altitude) {
        d->defaultAltitude = altitude;
        emit changed();
    }
}

int KmlLocationImportSettings::getDefaultIndicatedAirspeed() const noexcept
{
    return d->defaultIndicatedAirspeed;
}

void KmlLocationImportSettings::setDefaultIndicatedAirspeed(int indicatedAirspeed) noexcept
{
    if (d->defaultIndicatedAirspeed != indicatedAirspeed) {
        d->defaultIndicatedAirspeed = indicatedAirspeed;
        emit changed();
    }
}

// PROTECTED

void KmlLocationImportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

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

void KmlLocationImportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

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

void KmlLocationImportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};

    const std::int64_t defaultCountryId = valuesByKey.at(::DefaultCountryKey).toLongLong(&ok);
    d->defaultCountryId = ok ? defaultCountryId : d->WorldCountryId;

    const int defaultAltitude = valuesByKey.at(::DefaultAltitudeKey).toInt(&ok);
    d->defaultAltitude = ok ? defaultAltitude : Const::DefaultAltitude;

    const int defaultIndicatedAirspeed = valuesByKey.at(::DefaultIndicatedAirspeedKey).toInt(&ok);
    d->defaultIndicatedAirspeed = ok ? defaultIndicatedAirspeed : Const::DefaultIndicatedAirspeed;
}

void KmlLocationImportSettings::restoreDefaultsExtn() noexcept
{
    d->defaultCountryId = d->WorldCountryId;
    d->defaultAltitude = Const::DefaultAltitude;
    d->defaultIndicatedAirspeed = Const::DefaultIndicatedAirspeed;
}
