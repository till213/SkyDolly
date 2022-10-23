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

#include <Kernel/CsvParser.h>
#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/Service/EnumerationService.h>
#include "LittleNavmapCsvParser.h"

namespace
{
    // Also refer to Locations.csv
    constexpr int TypeIndex = 0;
    constexpr int TitleIndex = 1;
    constexpr int IdentIndex = 2;
    constexpr int LatitudeIndex = 3;
    constexpr int LongitudeIndex = 3;
    constexpr int ElevationIndex = 4;
    constexpr int DescriptionIndex = 8;

    constexpr const char *LittleNavmapCsvHeader {"type,name,ident,latitude,longitude,elevation"};
}

struct LittleNavmapCsvParserPrivate
{
public:
    LittleNavmapCsvParserPrivate() noexcept
    {
        initTypeToSymbolicIdMap();
        Enumeration locationType = enumerationService.getEnumerationByName(EnumerationService::LocationType);
        locationTypeImportId = locationType.getItemBySymbolicId(EnumerationService::LocationTypeImportSymbolicId).id;
    }

    EnumerationService enumerationService;
    // Key: Litte Navmap userpoint type, value: symbolic category ID
    std::unordered_map<QString, QString> typeToSymbolicId;
    std::int64_t locationTypeImportId;

private:
    inline void initTypeToSymbolicIdMap() {
        typeToSymbolicId["airprort"] = "AP";
        typeToSymbolicId["airstrip"] = "AS";
        typeToSymbolicId["building"] = "BU";
        typeToSymbolicId["helipad"] = "HP";
        typeToSymbolicId["history"] = "PO";
        typeToSymbolicId["landform"] = "LM";
        typeToSymbolicId["lighthouse"] = "LH";
        typeToSymbolicId["location"] = "PO";
        typeToSymbolicId["marker"] = "PO";
        typeToSymbolicId["mountain"] = "MO";
        typeToSymbolicId["other"] = "OT";
        typeToSymbolicId["park"] = "PA";
        typeToSymbolicId["pin"] = "PO";
        typeToSymbolicId["poi"] = "PO";
        typeToSymbolicId["seaport"] = "SP";
        typeToSymbolicId["settlement"] = "CI";
        typeToSymbolicId["water"] = "LA";
    }
};

// PUBLIC

LittleNavmapCsvParser::LittleNavmapCsvParser() noexcept
    : d(std::make_unique<LittleNavmapCsvParserPrivate>())
{}

LittleNavmapCsvParser::LittleNavmapCsvParser(LittleNavmapCsvParser &&rhs) = default;
LittleNavmapCsvParser &LittleNavmapCsvParser::operator=(LittleNavmapCsvParser &&rhs) = default;
LittleNavmapCsvParser::~LittleNavmapCsvParser() = default;

std::vector<Location> LittleNavmapCsvParser::parse(QTextStream &textStream, bool *ok) noexcept
{
    std::vector<Location> locations;
    CsvParser csvParser;

    bool success {true};
    CsvParser::Rows rows = csvParser.parse(textStream, ::LittleNavmapCsvHeader);
    locations.reserve(rows.size());
    for (const auto &columns : rows) {
        const Location location = parseLocation(columns, success);
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

Location LittleNavmapCsvParser::parseLocation(CsvParser::Columns columns, bool &ok) const noexcept
{
    Location location;

    ok = true;
    location.title = columns.at(::TitleIndex);
    location.typeId = d->locationTypeImportId;
    if (ok) {
        const QString type = columns.at(::TypeIndex);
        location.categoryId = mapTypeToCategoryId(type);
    }
    if (ok) {
        location.identifier = columns.at(::IdentIndex);
    }
    if (ok) {
        const double latitude = columns.at(::LatitudeIndex).toDouble(&ok);
        if (ok) {
            location.latitude = latitude;
        }
    }
    if (ok) {
        const double longitude = columns.at(::LongitudeIndex).toDouble(&ok);
        if (ok) {
            location.longitude = longitude;
        }
    }
    if (ok) {
        const double altitude = columns.at(::ElevationIndex).toDouble(&ok);
        if (ok) {
            location.altitude = altitude;
        }
    }
    if (ok) {
        location.description = columns.at(::DescriptionIndex);
    }

    return location;
}

inline std::int64_t LittleNavmapCsvParser::mapTypeToCategoryId(const QString &type) const noexcept
{
    Enumeration locationCategory = d->enumerationService.getEnumerationByName(EnumerationService::LocationCategory);
    QString categorySymbolicId;
    const auto it = d->typeToSymbolicId.find(type.toLower());
    if (it != d->typeToSymbolicId.end()) {
       categorySymbolicId = it->second;
    } else {
        categorySymbolicId = EnumerationService::LocationCategoryNoneSymbolicId;
    }
    return locationCategory.getItemBySymbolicId(categorySymbolicId).id;
}
