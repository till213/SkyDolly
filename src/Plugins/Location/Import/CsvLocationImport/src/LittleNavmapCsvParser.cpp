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

#include <QtGlobal>
#include <QIODevice>
#include <QTextStream>
#include <QTextCodec>

#include <Kernel/CsvParser.h>
#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/Service/EnumerationService.h>
#include "CsvLocationImportSettings.h"
#include "LittleNavmapCsvParser.h"

namespace
{
    enum Index
    {
        Type = 0,
        Title,
        Ident,
        Latitude,
        Longitude,
        Elevation,
        Description
    };

    constexpr const char *LittleNavmapCsvHeader {"type,name,ident,latitude,longitude,elevation"};
}

struct LittleNavmapCsvParserPrivate
{
public:
    LittleNavmapCsvParserPrivate(const CsvLocationImportSettings &pluginSettings) noexcept
        : pluginSettings(pluginSettings)
    {
        initTypeToSymbolicIdMap();
    }

    const CsvLocationImportSettings &pluginSettings;
    const std::int64_t ImportTypeId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::LocationTypeImportSymId).id()};
    const std::int64_t KeepEngineEventId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventKeepSymId).id()};

    // Key: Litte Navmap userpoint type, value: symbolic category ID
    std::unordered_map<QString, QString> typeToSymbolicId;

private:
    inline void initTypeToSymbolicIdMap() {
        typeToSymbolicId["airport"] = "AP";
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

LittleNavmapCsvParser::LittleNavmapCsvParser(const CsvLocationImportSettings &pluginSettings) noexcept
    : d(std::make_unique<LittleNavmapCsvParserPrivate>(pluginSettings))
{}

LittleNavmapCsvParser::LittleNavmapCsvParser(LittleNavmapCsvParser &&rhs) noexcept = default;
LittleNavmapCsvParser &LittleNavmapCsvParser::operator=(LittleNavmapCsvParser &&rhs) noexcept = default;
LittleNavmapCsvParser::~LittleNavmapCsvParser() = default;

std::vector<Location> LittleNavmapCsvParser::parse(QTextStream &textStream, bool *ok) noexcept
{
    std::vector<Location> locations;
    CsvParser csvParser;

    bool success {true};
    CsvParser::Rows rows = csvParser.parse(textStream, ::LittleNavmapCsvHeader);
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

Location LittleNavmapCsvParser::parseLocation(CsvParser::Row row, bool &ok) const noexcept
{
    Location location;

    ok = true;
    location.title = row.at(::Index::Title);
    location.countryId = d->pluginSettings.getDefaultCountryId();
    location.typeId = d->ImportTypeId;
    location.engineEventId = d->KeepEngineEventId;
    const QString type = row.at(::Index::Type);
    location.categoryId = mapTypeToCategoryId(type);
    location.identifier = row.at(::Index::Ident);
    const double latitude = row.at(::Index::Latitude).toDouble(&ok);
    if (ok) {
        location.latitude = latitude;
    }
    if (ok) {
        const double longitude = row.at(::Index::Longitude).toDouble(&ok);
        if (ok) {
            location.longitude = longitude;
        }
    }
    if (ok) {
        const QVariant data = row.at(::Index::Elevation);
        if (!data.isNull()) {
            const double altitude = data.toDouble(&ok);
            if (ok) {
                if (!qFuzzyIsNull(altitude)) {
                    location.altitude = altitude;
                } else {
                    // TODO Provide an option to:
                    // - place aircraft on ground
                    // - use default altitude
                    // - import "as is" (0.0 feet)
                    location.altitude = d->pluginSettings.getDefaultAltitude();
                }
            }
        } else {
            location.altitude = d->pluginSettings.getDefaultAltitude();
        }
    }
    if (ok) {
        location.indicatedAirspeed = d->pluginSettings.getDefaultIndicatedAirspeed();
    }
    if (ok) {
        location.description = row.at(::Index::Description);
    }

    return location;
}

inline std::int64_t LittleNavmapCsvParser::mapTypeToCategoryId(const QString &type) const noexcept
{
    EnumerationService enumerationService;
    Enumeration locationCategory = enumerationService.getEnumerationByName(EnumerationService::LocationCategory);
    QString categorySymbolicId;
    const auto it = d->typeToSymbolicId.find(type.toLower());
    if (it != d->typeToSymbolicId.end()) {
       categorySymbolicId = it->second;
    } else {
        categorySymbolicId = EnumerationService::LocationCategoryNoneSymId;
    }
    return locationCategory.getItemBySymbolicId(categorySymbolicId).id;
}
