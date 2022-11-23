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
    enum Index
    {
        Title = 0,
        Description,
        Type,
        Category,
        Country,
        Attributes,
        Identifier,
        Latitude,
        Longitude,
        Altitude,
        Pitch,
        Bank,
        TrueHeading,
        IndicatedAirspeed,
        OnGround,
        EngineEvent
    };

    constexpr const char *SkyDollyCsvHeader {"Title,Description,Type,Category,Country,Attributes"};
}

struct SkyDollyCsvLocationParserPrivate
{
    EnumerationService enumerationService;
    Enumeration typeEnumeration {enumerationService.getEnumerationByName(EnumerationService::LocationType)};
    Enumeration categoryEnumeration {enumerationService.getEnumerationByName(EnumerationService::LocationCategory)};
    Enumeration countryEnumeration {enumerationService.getEnumerationByName(EnumerationService::Country)};
    Enumeration engineEventEnumeration {enumerationService.getEnumerationByName(EnumerationService::EngineEvent)};
    std::int64_t importTypeId {typeEnumeration.getItemBySymbolicId(EnumerationService::LocationTypeImportSymId).id};
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
    location.title = row.at(::Index::Title);
    location.description = row.at(::Description);
    // For now imported Sky Dolly locations always are of type "imported"
    location.typeId = d->importTypeId;
    ok = location.typeId != Const::InvalidId;
    if (ok) {
        const QString categorySymbolicId = row.at(::Index::Category);
        location.categoryId = d->categoryEnumeration.getItemBySymbolicId(categorySymbolicId).id;
        ok = location.categoryId != Const::InvalidId;
    }
    if (ok) {
        const QString countrySymbolicId = row.at(::Index::Country);
        location.countryId = d->countryEnumeration.getItemBySymbolicId(countrySymbolicId).id;
        ok = location.countryId != Const::InvalidId;
    }
    if (ok) {
        const std::int64_t attributes = row.at(::Index::Attributes).toLongLong(&ok);
        if (ok) {
            location.attributes = attributes;
        }
    }
    if (ok) {
        location.identifier = row.at(::Index::Identifier);
    }
    if (ok) {
        const double latitude = row.at(::Index::Latitude).toDouble(&ok);
        if (ok) {
            location.latitude = latitude;
        }
    }
    if (ok) {
        const double longitude = row.at(::Index::Longitude).toDouble(&ok);
        if (ok) {
            location.longitude = longitude;
        }
    }
    if (ok) {
        const double altitude = row.at(::Index::Altitude).toDouble(&ok);
        if (ok) {
            location.altitude = altitude;
        }
    }
    if (ok) {
        const double pitch = row.at(::Index::Pitch).toDouble(&ok);
        if (ok) {
            location.pitch = pitch;
        }
    }
    if (ok) {
        const double bank = row.at(::Index::Bank).toDouble(&ok);
        if (ok) {
            location.bank = bank;
        }
    }
    if (ok) {
        const double trueHeading = row.at(::Index::TrueHeading).toDouble(&ok);
        if (ok) {
            location.trueHeading = trueHeading;
        }
    }
    if (ok) {
        const int indicatedAirspeed = row.at(::Index::IndicatedAirspeed).toInt(&ok);
        if (ok) {
            location.indicatedAirspeed = indicatedAirspeed;
        }
    }
    if (ok) {
        const QString onGround = row.at(::Index::OnGround);
        if (ok) {
            location.onGround = onGround == "true" ? true : false;
        }
    }
    if (ok) {
        const QString engineEventSymbolicId = row.at(::Index::EngineEvent);
        location.engineEventId = d->engineEventEnumeration.getItemBySymbolicId(engineEventSymbolicId).id;
        ok = location.engineEventId != Const::InvalidId;
    }

    return location;
}
