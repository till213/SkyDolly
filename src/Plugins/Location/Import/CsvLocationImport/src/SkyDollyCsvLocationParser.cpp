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
#include <cstdint>

#include <QIODevice>
#include <QTextStream>
#include <QString>
#include <QStringConverter>
#include <QDate>
#include <QTime>

#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include <Kernel/CsvParser.h>
#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/Service/EnumerationService.h>
#include <PluginManager/Csv.h>
#include "SkyDollyCsvLocationParser.h"

namespace
{
    enum struct Index: std::uint8_t
    {
        Title = 0,
        Description,
        Type,
        Category,
        Country,
        Identifier,
        Latitude,
        Longitude,
        Altitude,
        Pitch,
        Bank,
        TrueHeading,
        IndicatedAirspeed,
        OnGround,
        EngineEvent,
        LocalSimulationDate,
        LocalSimulationTime,
        // Last index
        Count
    };

    constexpr const char *SkyDollyCsvHeader {"Title,Description,Type,Category,Country,Identifier"};
}

struct SkyDollyCsvLocationParserPrivate
{
    EnumerationService enumerationService;
    Enumeration typeEnumeration {enumerationService.getEnumerationByName(EnumerationService::LocationType)};
    Enumeration categoryEnumeration {enumerationService.getEnumerationByName(EnumerationService::LocationCategory)};
    Enumeration countryEnumeration {enumerationService.getEnumerationByName(EnumerationService::Country)};
    Enumeration engineEventEnumeration {enumerationService.getEnumerationByName(EnumerationService::EngineEvent)};
    std::int64_t importTypeId {typeEnumeration.getItemBySymId(EnumerationService::LocationTypeImportSymId).id};
};

// PUBLIC

SkyDollyCsvLocationParser::SkyDollyCsvLocationParser() noexcept
    : d {std::make_unique<SkyDollyCsvLocationParserPrivate>()}
{}

SkyDollyCsvLocationParser::SkyDollyCsvLocationParser(SkyDollyCsvLocationParser &&rhs) noexcept = default;
SkyDollyCsvLocationParser &SkyDollyCsvLocationParser::operator=(SkyDollyCsvLocationParser &&rhs) noexcept = default;
SkyDollyCsvLocationParser::~SkyDollyCsvLocationParser() = default;

std::vector<Location> SkyDollyCsvLocationParser::parse(QTextStream &textStream, bool *ok) noexcept
{
    std::vector<Location> locations;
    CsvParser csvParser;

    CsvParser::Rows rows = csvParser.parse(textStream, ::SkyDollyCsvHeader);
    bool success = CsvParser::validate(rows, Enum::underly(::Index::Count));
    if (success) {
        locations.reserve(rows.size());
        for (const auto &row : rows) {
            const Location location = parseLocation(row, success);
            if (success) {
                locations.push_back(location);
            } else {
                break;
            }
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
    location.title = row.at(Enum::underly(::Index::Title));
    location.description = row.at(Enum::underly(::Index::Description));
    // For now imported Sky Dolly locations always are of type "imported"
    location.typeId = d->importTypeId;
    ok = location.typeId != Const::InvalidId;
    if (ok) {
        const auto categorySymId = row.at(Enum::underly(::Index::Category));
        location.categoryId = d->categoryEnumeration.getItemBySymId(categorySymId).id;
        ok = location.categoryId != Const::InvalidId;
    }
    if (ok) {
        const auto countrySymId = row.at(Enum::underly(::Index::Country));
        location.countryId = d->countryEnumeration.getItemBySymId(countrySymId).id;
        ok = location.countryId != Const::InvalidId;
    }
    if (ok) {
        location.identifier = row.at(Enum::underly(::Index::Identifier));
    }
    if (ok) {
        const auto latitude = row.at(Enum::underly(::Index::Latitude)).toDouble(&ok);
        if (ok) {
            location.latitude = latitude;
        }
    }
    if (ok) {
        const auto longitude = row.at(Enum::underly(::Index::Longitude)).toDouble(&ok);
        if (ok) {
            location.longitude = longitude;
        }
    }
    if (ok) {
        const auto altitude = row.at(Enum::underly(::Index::Altitude)).toDouble(&ok);
        if (ok) {
            location.altitude = altitude;
        }
    }
    if (ok) {
        const auto pitch = row.at(Enum::underly(::Index::Pitch)).toDouble(&ok);
        if (ok) {
            location.pitch = pitch;
        }
    }
    if (ok) {
        const auto bank = row.at(Enum::underly(::Index::Bank)).toDouble(&ok);
        if (ok) {
            location.bank = bank;
        }
    }
    if (ok) {
        const auto trueHeading = row.at(Enum::underly(::Index::TrueHeading)).toDouble(&ok);
        if (ok) {
            location.trueHeading = trueHeading;
        }
    }
    if (ok) {
        const auto indicatedAirspeed = row.at(Enum::underly(::Index::IndicatedAirspeed)).toInt(&ok);
        if (ok) {
            location.indicatedAirspeed = indicatedAirspeed;
        }
    }
    if (ok) {
        const auto onGround = row.at(Enum::underly(::Index::OnGround));
        if (ok) {
            location.onGround = onGround == "true" ? true : false;
        }
    }
    if (ok) {
        const auto engineEventSymId = row.at(Enum::underly(::Index::EngineEvent));
        location.engineEventId = d->engineEventEnumeration.getItemBySymId(engineEventSymId).id;
        ok = location.engineEventId != Const::InvalidId;
    }
    if (ok) {
        const auto localSimulationDateValue = row.at(Enum::underly(::Index::LocalSimulationDate));
        if (!localSimulationDateValue.isEmpty()) {
            const auto localSimulationDate = QDate::fromString(localSimulationDateValue, Qt::DateFormat::ISODate);
            ok = localSimulationDate.isValid();
            if (ok) {
                location.localSimulationDate = localSimulationDate;
            }
        }
    }
    if (ok) {
        const auto localSimulationTimeValue = row.at(Enum::underly(::Index::LocalSimulationTime));
        if (!localSimulationTimeValue.isEmpty()) {
            const auto localSimulationTime = QTime::fromString(localSimulationTimeValue, Qt::DateFormat::ISODate);
            ok = localSimulationTime.isValid();
            if (ok) {
                location.localSimulationTime = localSimulationTime;
            }
        }
    }

    return location;
}
