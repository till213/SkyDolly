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
#include <memory>
#include <cstdint>
#include <unordered_map>

#include <QtGlobal>
#include <QIODevice>
#include <QTextStream>

#include <Kernel/CsvParser.h>
#include <Kernel/Enum.h>
#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/Service/EnumerationService.h>
#include <PluginManager/Csv.h>
#include "CsvLocationImportSettings.h"
#include "LittleNavmapCsvParser.h"

namespace Header
{
    constexpr const char *LittleNavmap {"Type,Name,Ident,Latitude,Longitude,Elevation,Magnetic Declination,Tags,Description,Region,Visible From,Last Edit,Import Filename"};

    // Column names (also add them to LittleNavmapCsvParserPrivate::HeaderNames, for validation)
    constexpr const char *Type {"Type"};
    constexpr const char *Name {"Name"};
    constexpr const char *Ident {"Ident"};
    constexpr const char *Latitude {"Latitude"};
    constexpr const char *Longitude {"Longitude"};
    constexpr const char *Elevation {"Elevation"};
    constexpr const char *MagneticDeclination {"Magnetic Declination"};
    constexpr const char *Tags {"Tags"};
    constexpr const char *Description {"Description"};
}

struct LittleNavmapCsvParserPrivate
{
    LittleNavmapCsvParserPrivate(const CsvLocationImportSettings &pluginSettings) noexcept
        : pluginSettings {pluginSettings}
    {
        initTypeToSymIdMap();
    }

    const CsvLocationImportSettings &pluginSettings;
    const std::int64_t ImportTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeImportSymId).id()};
    const std::int64_t KeepEngineEventId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventKeepSymId).id()};

    // Key: Litte Navmap userpoint type, value: symbolic category ID
    std::unordered_map<QString, QString> typeToSymId;
    CsvParser::Headers headers;

    static constexpr std::array<const char *, 9> HeaderNames {
        Header::Type,
        Header::Name,
        Header::Ident,
        Header::Latitude,
        Header::Longitude,
        Header::Elevation,
        Header::MagneticDeclination,
        Header::Tags,
        Header::Description
    };

private:
    inline void initTypeToSymIdMap() {
        typeToSymId[QStringLiteral("addon")] = QStringLiteral("AP");
        typeToSymId[QStringLiteral("airport")] = QStringLiteral("AP");
        typeToSymId[QStringLiteral("airstrip")] = QStringLiteral("AS");
        typeToSymId[QStringLiteral("building")] = QStringLiteral("BU");
        typeToSymId[QStringLiteral("cabin")] = QStringLiteral("CB");
        typeToSymId[QStringLiteral("helipad")] = QStringLiteral("HP");
        typeToSymId[QStringLiteral("history")] = QStringLiteral("HI");
        typeToSymId[QStringLiteral("landform")] = QStringLiteral("LM");
        typeToSymId[QStringLiteral("lighthouse")] = QStringLiteral("LH");
        typeToSymId[QStringLiteral("location")] = QStringLiteral("PO");
        typeToSymId[QStringLiteral("marker")] = QStringLiteral("PO");
        typeToSymId[QStringLiteral("mountain")] = QStringLiteral("MO");
        typeToSymId[QStringLiteral("obstacle")] = QStringLiteral("OB");
        typeToSymId[QStringLiteral("other")] = QStringLiteral("OT");
        typeToSymId[QStringLiteral("oil platform")] = QStringLiteral("OP");
        typeToSymId[QStringLiteral("park")] = QStringLiteral("PA");
        typeToSymId[QStringLiteral("pin")] = QStringLiteral("PO");
        typeToSymId[QStringLiteral("poi")] = QStringLiteral("PO");
        typeToSymId[QStringLiteral("seaport")] = QStringLiteral("SP");
        typeToSymId[QStringLiteral("settlement")] = QStringLiteral("ST");
        typeToSymId[QStringLiteral("water")] = QStringLiteral("LA");
    }
};

// PUBLIC

LittleNavmapCsvParser::LittleNavmapCsvParser(const CsvLocationImportSettings &pluginSettings) noexcept
    : d {std::make_unique<LittleNavmapCsvParserPrivate>(pluginSettings)}
{}

LittleNavmapCsvParser::LittleNavmapCsvParser(LittleNavmapCsvParser &&rhs) noexcept = default;
LittleNavmapCsvParser &LittleNavmapCsvParser::operator=(LittleNavmapCsvParser &&rhs) noexcept = default;
LittleNavmapCsvParser::~LittleNavmapCsvParser() = default;

std::vector<Location> LittleNavmapCsvParser::parse(QTextStream &textStream, bool *ok) noexcept
{
    std::vector<Location> locations;
    CsvParser csvParser;

    CsvParser::Rows rows = csvParser.parse(textStream, Header::LittleNavmap);
    d->headers = csvParser.getHeaders();
    bool success = validateHeaders();
    if (success) {
        success = CsvParser::validate(rows, d->headers.size());
    }
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

// PRIVATE

bool LittleNavmapCsvParser::validateHeaders() const noexcept
{
    bool ok {true};
    for (const auto val : d->HeaderNames) {
        ok = d->headers.contains(val);
        if (!ok) {
            break;
        }
    }
    return ok;
}

Location LittleNavmapCsvParser::parseLocation(CsvParser::Row row, bool &ok) const noexcept
{
    Location location;

    ok = true;
    location.title = row.at(d->headers.at(Header::Name));
    location.countryId = d->pluginSettings.getDefaultCountryId();
    location.typeId = d->ImportTypeId;
    location.engineEventId = d->KeepEngineEventId;
    const auto type = row.at(d->headers.at(Header::Type));
    location.categoryId = mapTypeToCategoryId(type);
    location.identifier = row.at(d->headers.at(Header::Ident));
    const auto latitude = row.at(d->headers.at(Header::Latitude)).toDouble(&ok);
    if (ok) {
        location.latitude = latitude;
    }
    if (ok) {
        const auto longitude = row.at(d->headers.at(Header::Longitude)).toDouble(&ok);
        if (ok) {
            location.longitude = longitude;
        }
    }
    if (ok) {
        const QVariant data = row.at(d->headers.at(Header::Elevation));
        if (!data.isNull() && !data.toString().isEmpty()) {
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
        location.description = row.at(d->headers.at(Header::Description));
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
