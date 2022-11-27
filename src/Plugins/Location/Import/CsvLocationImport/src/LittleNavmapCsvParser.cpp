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
#include <Kernel/Enum.h>
#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/Service/EnumerationService.h>
#include <PluginManager/Csv.h>
#include "CsvLocationImportSettings.h"
#include "LittleNavmapCsvParser.h"

namespace
{
    enum struct Index
    {
        Type = 0,
        Name,
        Ident,
        Latitude,
        Longitude,
        Elevation,
        MagneticDeclination,
        Tags,
        Description,
        Region,
        VisibleFrom,
        LastEdit,
        ImportFilename,
        // Last index
        Count
    };

    constexpr const char *LittleNavmapCsvHeader {"type,name,ident,latitude,longitude,elevation"};
}

struct LittleNavmapCsvParserPrivate
{
public:
    LittleNavmapCsvParserPrivate(const CsvLocationImportSettings &pluginSettings) noexcept
        : pluginSettings(pluginSettings)
    {
        initTypeToSymIdMap();
    }

    const CsvLocationImportSettings &pluginSettings;
    const std::int64_t ImportTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeImportSymId).id()};
    const std::int64_t KeepEngineEventId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventKeepSymId).id()};

    // Key: Litte Navmap userpoint type, value: symbolic category ID
    std::unordered_map<QString, QString> typeToSymId;

private:
    inline void initTypeToSymIdMap() {
        typeToSymId["airport"] = "AP";
        typeToSymId["airstrip"] = "AS";
        typeToSymId["building"] = "BU";
        typeToSymId["helipad"] = "HP";
        typeToSymId["history"] = "PO";
        typeToSymId["landform"] = "LM";
        typeToSymId["lighthouse"] = "LH";
        typeToSymId["location"] = "PO";
        typeToSymId["marker"] = "PO";
        typeToSymId["mountain"] = "MO";
        typeToSymId["other"] = "OT";
        typeToSymId["park"] = "PA";
        typeToSymId["pin"] = "PO";
        typeToSymId["poi"] = "PO";
        typeToSymId["seaport"] = "SP";
        typeToSymId["settlement"] = "CI";
        typeToSymId["water"] = "LA";
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

    CsvParser::Rows rows = csvParser.parse(textStream, ::LittleNavmapCsvHeader);
    bool success = Csv::validate(rows, Enum::underly(::Index::Count));
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

Location LittleNavmapCsvParser::parseLocation(CsvParser::Row row, bool &ok) const noexcept
{
    Location location;

    ok = true;
    location.title = row.at(Enum::underly(::Index::Name));
    location.countryId = d->pluginSettings.getDefaultCountryId();
    location.typeId = d->ImportTypeId;
    location.engineEventId = d->KeepEngineEventId;
    const QString type = row.at(Enum::underly(::Index::Type));
    location.categoryId = mapTypeToCategoryId(type);
    location.identifier = row.at(Enum::underly(::Index::Ident));
    const double latitude = row.at(Enum::underly(::Index::Latitude)).toDouble(&ok);
    if (ok) {
        location.latitude = latitude;
    }
    if (ok) {
        const double longitude = row.at(Enum::underly(::Index::Longitude)).toDouble(&ok);
        if (ok) {
            location.longitude = longitude;
        }
    }
    if (ok) {
        const QVariant data = row.at(Enum::underly(::Index::Elevation));
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
        location.description = row.at(Enum::underly(::Index::Description));
    }

    return location;
}

inline std::int64_t LittleNavmapCsvParser::mapTypeToCategoryId(const QString &type) const noexcept
{
    EnumerationService enumerationService;
    Enumeration locationCategory = enumerationService.getEnumerationByName(EnumerationService::LocationCategory);
    QString categorySymId;
    const auto it = d->typeToSymId.find(type.toLower());
    if (it != d->typeToSymId.end()) {
       categorySymId = it->second;
    } else {
        categorySymId = EnumerationService::LocationCategoryNoneSymId;
    }
    return locationCategory.getItemBySymId(categorySymId).id;
}
