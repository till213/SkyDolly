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
#include <vector>

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
#include "SkyDollyCsvLocationWriter.h"

namespace
{
    constexpr const char *TitleColumn {"Title"};
    constexpr const char *DescriptionColumn {"Description"};
    constexpr const char *TypeColumn {"Type"};
    constexpr const char *CategoryColumn {"Category"};
    constexpr const char *CountryColumn {"Country"};
    constexpr const char *AttributesColumn {"Attributes"};
    constexpr const char *IdentifierColumn {"Identifier"};
    constexpr const char *LatitudeColumn {"Latitude"};
    constexpr const char *LongitudeColumn {"Longitude"};
    constexpr const char *AltitudeColumn {"Altitude"};
    constexpr const char *PitchColumn {"Pitch"};
    constexpr const char *BankColumn {"Bank"};
    constexpr const char *TrueHeadingColumn {"True Heading"};
    constexpr const char *IndicatedAirspeedColumn {"Indicated Airspeed"};
    constexpr const char *OnGroundColumn {"On Ground"};
    constexpr const char *EngineEventColumn {"Engine Event"};
}

struct SkyDollyCsvLocationWriterPrivate
{
    SkyDollyCsvLocationWriterPrivate(const CsvLocationExportSettings &thePluginSettings) noexcept
        : pluginSettings(thePluginSettings)
    {}

    const CsvLocationExportSettings &pluginSettings;
    EnumerationService enumerationService;
};

// PUBLIC

SkyDollyCsvLocationWriter::SkyDollyCsvLocationWriter(const CsvLocationExportSettings &pluginSettings) noexcept
    : d(std::make_unique<SkyDollyCsvLocationWriterPrivate>(pluginSettings))
{}

SkyDollyCsvLocationWriter::~SkyDollyCsvLocationWriter() = default;

bool SkyDollyCsvLocationWriter::write(const std::vector<Location> &locations, QIODevice &io) noexcept
{
    QString csv = QString::fromLatin1(::TitleColumn) % Csv::CommaSep %
                                      ::DescriptionColumn % Csv::CommaSep %
                                      ::TypeColumn % Csv::CommaSep %
                                      ::CategoryColumn % Csv::CommaSep %
                                      ::CountryColumn % Csv::CommaSep %
                                      ::AttributesColumn % Csv::CommaSep %
                                      ::IdentifierColumn % Csv::CommaSep %
                                      ::LatitudeColumn % Csv::CommaSep %
                                      ::LongitudeColumn % Csv::CommaSep %
                                      ::AltitudeColumn % Csv::CommaSep %
                                      ::PitchColumn % Csv::CommaSep %
                                      ::BankColumn % Csv::CommaSep %
                                      ::TrueHeadingColumn % Csv::CommaSep %
                                      ::IndicatedAirspeedColumn % Csv::CommaSep %
                                      ::OnGroundColumn % Csv::CommaSep %
                                      ::EngineEventColumn % Csv::Ln;

    bool ok = io.write(csv.toUtf8());
    Enumeration locationTypeEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::LocationType);
    Enumeration locationCategoryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::LocationCategory);
    Enumeration countryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::Country);
    Enumeration engineEventEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::EngineEvent);
    if (ok) {
        for (const auto &location : locations) {
            QString title = location.title;
            QString description = location.description;
            QString identifier = location.identifier;
            const QString locationTypeSymId = locationTypeEnumeration.getItemById(location.typeId).symId;
            const QString locationCategorySymId = locationCategoryEnumeration.getItemById(location.categoryId).symId;
            const QString countrySymId = countryEnumeration.getItemById(location.countryId).symId;
            const QString engineEventSymId = engineEventEnumeration.getItemById(location.engineEventId).symId;
            const QString csv = QString::fromLatin1("\"") %
                                title.replace(QString::fromLatin1("\""), QString::fromLatin1("\"\"")) % "\""  %
                                Csv::CommaSep % "\"" %
                                description.replace(QString::fromLatin1("\""), QString::fromLatin1("\"\"")) % "\"" % Csv::CommaSep %
                                locationTypeSymId % Csv::CommaSep %
                                locationCategorySymId % Csv::CommaSep %
                                countrySymId % Csv::CommaSep %
                                QString::number(location.attributes) % Csv::CommaSep %
                                "\"" % identifier.replace(QString::fromLatin1("\""), QString::fromLatin1("\"\"")) % "\"" % Csv::CommaSep %
                                Export::formatCoordinate(location.latitude) % Csv::CommaSep %
                                Export::formatCoordinate(location.longitude) % Csv::CommaSep %
                                Export::formatNumber(location.altitude) % Csv::CommaSep %
                                Export::formatNumber(location.pitch) % Csv::CommaSep %
                                Export::formatNumber(location.bank) % Csv::CommaSep %
                                Export::formatNumber(location.trueHeading) % Csv::CommaSep %
                                QString::number(location.indicatedAirspeed) % Csv::CommaSep %
                                (location.onGround ? "true" : "false") % Csv::CommaSep %
                                engineEventSymId % Csv::Ln;
            ok = io.write(csv.toUtf8());

            if (!ok) {
                break;
            }
        }
    }
    return ok;
}
