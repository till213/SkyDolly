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
#include <cstdint>
#include <unordered_map>

#include <QIODevice>
#include <QTextStream>
#include <QTextCodec>

#include <Kernel/Const.h>
#include <Kernel/CsvParser.h>
#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/Service/EnumerationService.h>
#include "SkyDollyCsvLocationParser.h"

namespace
{
    // Also refer to Locations.csv
    constexpr int TitleIndex = 0;
    constexpr int DescriptionIndex = 1;
    constexpr int TypeIndex = 2;
    constexpr int CategoryIndex = 3;
    constexpr int CountryIndex = 4;
    constexpr int AttributesIndex = 5;
    constexpr int IdentifierIndex = 6;
    constexpr int LatitudeIndex = 7;
    constexpr int LongitudeIndex = 8;
    constexpr int AltitudeIndex = 9;
    constexpr int PitchIndex = 10;
    constexpr int BankIndex = 11;
    constexpr int TrueHeadingIndex = 12;
    constexpr int IndicatedAirspeedIndex = 13;
    constexpr int OnGroundIndex = 14;

    constexpr const char *SkyDollyCsvHeader {"Title,Description,Type,Category,Country,Attributes"};
}

struct SkyDollyCsvLocationParserPrivate
{
public:
    EnumerationService enumerationService;
    Enumeration typeEnumeration {enumerationService.getEnumerationByName(EnumerationService::LocationType)};
    Enumeration categoryEnumeration {enumerationService.getEnumerationByName(EnumerationService::LocationCategory)};
    Enumeration countryEnumeration {enumerationService.getEnumerationByName(EnumerationService::Country)};
};

// PUBLIC

SkyDollyCsvLocationParser::SkyDollyCsvLocationParser() noexcept
    : d(std::make_unique<SkyDollyCsvLocationParserPrivate>())
{}

SkyDollyCsvLocationParser::SkyDollyCsvLocationParser(SkyDollyCsvLocationParser &&rhs) noexcept = default;
SkyDollyCsvLocationParser &SkyDollyCsvLocationParser::operator=(SkyDollyCsvLocationParser &&rhs) noexcept = default;
SkyDollyCsvLocationParser::~SkyDollyCsvLocationParser() = default;

std::vector<Location> SkyDollyCsvLocationParser::parse(QTextStream &textStream, bool *ok) noexcept
{
    std::vector<Location> locations;
    CsvParser csvParser;

    bool success {true};
    CsvParser::Rows rows = csvParser.parse(textStream, ::SkyDollyCsvHeader);
    locations.reserve(rows.size());
    for (const auto &row : rows) {
        const Location location = parseLocation(row, success);
        if (success) {
            locations.push_back(location);
        } else {
            break;
        }
    }

    if (ok != nullptr) {
        *ok = success;
    }

    return locations;
}

Location SkyDollyCsvLocationParser::parseLocation(CsvParser::Row row, bool &ok) const noexcept
{
    Location location;

    ok = true;
    location.title = row.at(::TitleIndex);
    location.description = row.at(::DescriptionIndex);
    const QString typeSymbolicId = row.at(::TypeIndex);
    location.typeId = d->typeEnumeration.getItemBySymbolicId(typeSymbolicId).id;
    ok = location.typeId != Const::InvalidId;
    if (ok) {
        const QString categorySymbolicId = row.at(::CategoryIndex);
        location.categoryId = d->categoryEnumeration.getItemBySymbolicId(categorySymbolicId).id;
        ok = location.categoryId != Const::InvalidId;
    }
    if (ok) {
        const QString countrySymbolicId = row.at(::CountryIndex);
        location.countryId = d->countryEnumeration.getItemBySymbolicId(countrySymbolicId).id;
        ok = location.countryId != Const::InvalidId;
    }
    if (ok) {
        const std::int64_t attributes = row.at(::AttributesIndex).toLongLong(&ok);
        if (ok) {
            location.attributes = attributes;
        }
    }
    if (ok) {
        location.identifier = row.at(::IdentifierIndex);
    }
    if (ok) {
        const double latitude = row.at(::LatitudeIndex).toDouble(&ok);
        if (ok) {
            location.latitude = latitude;
        }
    }
    if (ok) {
        const double longitude = row.at(::LongitudeIndex).toDouble(&ok);
        if (ok) {
            location.longitude = longitude;
        }
    }
    if (ok) {
        const double altitude = row.at(::AltitudeIndex).toDouble(&ok);
        if (ok) {
            location.altitude = altitude;
        }
    }
    if (ok) {
        const double pitch = row.at(::PitchIndex).toDouble(&ok);
        if (ok) {
            location.pitch = pitch;
        }
    }
    if (ok) {
        const double bank = row.at(::BankIndex).toDouble(&ok);
        if (ok) {
            location.bank = bank;
        }
    }
    if (ok) {
        const double trueHeading = row.at(::TrueHeadingIndex).toDouble(&ok);
        if (ok) {
            location.trueHeading = trueHeading;
        }
    }
    if (ok) {
        const int indicatedAirspeed = row.at(::IndicatedAirspeedIndex).toInt(&ok);
        if (ok) {
            location.indicatedAirspeed = indicatedAirspeed;
        }
    }
    if (ok) {
        const QString onGround = row.at(::OnGroundIndex);
        if (ok) {
            location.onGround = onGround == "true" ? true : false;
        }
    }

    return location;
}
