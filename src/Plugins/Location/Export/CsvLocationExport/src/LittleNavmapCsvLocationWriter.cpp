/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <vector>
#include <unordered_map>

#include <QIODevice>
#include <QChar>
#include <QString>
#include <QStringBuilder>

#include <Kernel/Enum.h>
#include <Kernel/Unit.h>
#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/Service/EnumerationService.h>
#include <PluginManager/Csv.h>
#include <PluginManager/Export.h>
#include "CsvLocationExportSettings.h"
#include "LittleNavmapCsvLocationWriter.h"

namespace
{
    constexpr const char *TypeColumn {"Type"};
    constexpr const char *NameColumn {"Name"};
    constexpr const char *IdentColumn {"Ident"};
    constexpr const char *LatitudeColumn {"Latitude"};
    constexpr const char *LongitudeColumn {"Longitude"};
    constexpr const char *ElevationColumn {"Elevation"};
    constexpr const char *MagneticDeclinationColumn {"Magnetic Declination"};
    constexpr const char *TagsColumn {"Tags"};
    constexpr const char *DescriptionColumn {"Description"};
    constexpr const char *RegionColumn {"Region"};
    constexpr const char *VisibleFromColumn {"Visible From"};
    constexpr const char *LastEditColumn {"Last Edit"};
    constexpr const char *ImportFilenameColumn {"Import Filename"};

    constexpr const char *OtherType = "Other";
}

struct LittleNavmapCsvLocationWriterPrivate
{
    LittleNavmapCsvLocationWriterPrivate(const CsvLocationExportSettings &thePluginSettings) noexcept
        : pluginSettings(thePluginSettings)
    {
        initSymIdToType();
    }

    const CsvLocationExportSettings &pluginSettings;
    EnumerationService enumerationService;
    // Key: symbolic category ID, value: Litte Navmap userpoint type
    std::unordered_map<QString, QString> symIdToType;

private:
    inline void initSymIdToType() {
        symIdToType["00"] = ::OtherType;
        symIdToType["AP"] = "Airport";
        symIdToType["AS"] = "Airstrip";
        symIdToType["BR"] = "POI";
        symIdToType["BU"] = "Building";
        symIdToType["CA"] = "Landform";
        symIdToType["CB"] = "Cabin";
        symIdToType["CI"] = "Settlement";
        symIdToType["CR"] = ::OtherType;
        symIdToType["DA"] = "POI";
        symIdToType["DE"] = "Landform";
        symIdToType["GL"] = "Mountain";
        symIdToType["HP"] = "Helipad";
        symIdToType["IS"] = "Landform";
        symIdToType["HI"] = "History";
        symIdToType["LA"] = "Water";
        symIdToType["LM"] = "Landform";
        symIdToType["LH"] = "Lighthouse";
        symIdToType["MO"] = "Mountain";
        symIdToType["OB"] = "Obstacle";
        symIdToType["OP"] = "Oil Platform";
        symIdToType["OT"] = ::OtherType;
        symIdToType["PA"] = "Park";
        symIdToType["PO"] = "POI";
        symIdToType["SE"] = "Water";
        symIdToType["ST"] = "Settlement";
        symIdToType["SP"] = "Seaport";
        symIdToType["TO"] = "Settlement";
        symIdToType["RI"] = "Water";
        symIdToType["VA"] = "POI";
        symIdToType["WA"] = "Water";
    }
};

// PUBLIC

LittleNavmapCsvLocationWriter::LittleNavmapCsvLocationWriter(const CsvLocationExportSettings &pluginSettings) noexcept
    : d {std::make_unique<LittleNavmapCsvLocationWriterPrivate>(pluginSettings)}
{}

LittleNavmapCsvLocationWriter::~LittleNavmapCsvLocationWriter() = default;

bool LittleNavmapCsvLocationWriter::write(const std::vector<Location> &locations, QIODevice &io) noexcept
{
    QString csv = QString::fromLatin1(::TypeColumn) % Csv::CommaSep
                                      % ::NameColumn % Csv::CommaSep
                                      % ::IdentColumn % Csv::CommaSep
                                      % ::LatitudeColumn % Csv::CommaSep
                                      % ::LongitudeColumn % Csv::CommaSep
                                      % ::ElevationColumn % Csv::CommaSep
                                      % ::MagneticDeclinationColumn % Csv::CommaSep
                                      % ::TagsColumn % Csv::CommaSep
                                      % ::DescriptionColumn % Csv::CommaSep
                                      % ::RegionColumn % Csv::CommaSep
                                      % ::VisibleFromColumn % Csv::CommaSep
                                      % ::LastEditColumn % Csv::CommaSep
                                      % ::ImportFilenameColumn % Csv::Ln;

    bool ok = io.write(csv.toUtf8());
    Enumeration locationCategoryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::LocationCategory);
    Enumeration countryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::Country);
    if (ok) {
        for (const auto &location : locations) {
            QString title = location.title;
            QString description = location.description;
            QString identifier = location.identifier;
            const QString categorySymId = locationCategoryEnumeration.getItemById(location.categoryId).symId;
            const QString type = mapCategorySymIdToType(categorySymId);
            const QString countrySymId = countryEnumeration.getItemById(location.countryId).symId;
            const QString csv = type % Csv::CommaSep % "\""
                                % title.replace("\"", "\"\"") % "\"" % Csv::CommaSep % "\""
                                % identifier.replace("\"", "\"\"") % "\"" % Csv::CommaSep
                                % Export::formatCoordinate(location.latitude) % Csv::CommaSep 
                                % Export::formatCoordinate(location.longitude) % Csv::CommaSep 
                                % Export::formatNumber(location.altitude) % Csv::CommaSep 
                                % "" % Csv::CommaSep 
                                % "" % Csv::CommaSep 
                                % "\"" % description.replace("\"", "\"\"") % "\"" % Csv::CommaSep
                                % countrySymId % Csv::CommaSep % ""
                                % Csv::CommaSep % "" % Csv::CommaSep
                                % "" % Csv::Ln;
            ok = io.write(csv.toUtf8());
            if (!ok) {
                break;
            }
        }
    }
    return ok;
}

// PRIVATE

inline QString LittleNavmapCsvLocationWriter::mapCategorySymIdToType(const QString &categorySymId) const noexcept
{
    QString type;
    const auto it = d->symIdToType.find(categorySymId);
    if (it != d->symIdToType.end()) {
       type = it->second;
    } else {
        type = ::OtherType;
    }
    return type;
}
