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
#include <PluginManager/CsvConst.h>
#include <PluginManager/Export.h>
#include "CsvLocationExportSettings.h"
#include "SkyDollyCsvLocationWriter.h"

namespace
{
    const QString TitleColumn = QStringLiteral("Title");
    const QString DescriptionColumn = QStringLiteral("Description");
    const QString TypeColumn = QStringLiteral("Type");
    const QString CategoryColumn = QStringLiteral("Category");
    const QString CountryColumn = QStringLiteral("Country");
    const QString AttributesColumn = QStringLiteral("Attributes");
    const QString IdentifierColumn = QStringLiteral("Identifier");
    const QString LatitudeColumn = QStringLiteral("Latitude");
    const QString LongitudeColumn = QStringLiteral("Longitude");
    const QString AltitudeColumn = QStringLiteral("Altitude");
    const QString PitchColumn = QStringLiteral("Pitch");
    const QString BankColumn = QStringLiteral("Bank");
    const QString TrueHeadingColumn = QStringLiteral("True Heading");
    const QString IndicatedAirspeedColumn = QStringLiteral("Indicated Airspeed");
    const QString OnGroundColumn = QStringLiteral("On Ground");
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

SkyDollyCsvLocationWriter::~SkyDollyCsvLocationWriter() noexcept = default;

bool SkyDollyCsvLocationWriter::write(const std::vector<Location> &locations, QIODevice &io) noexcept
{
    QString csv = QString(::TitleColumn % CsvConst::CommaSep %
                          ::DescriptionColumn % CsvConst::CommaSep %
                          ::TypeColumn % CsvConst::CommaSep %
                          ::CategoryColumn % CsvConst::CommaSep %
                          ::CountryColumn % CsvConst::CommaSep %
                          ::AttributesColumn % CsvConst::CommaSep %
                          ::IdentifierColumn % CsvConst::CommaSep %
                          ::LatitudeColumn % CsvConst::CommaSep %
                          ::LongitudeColumn % CsvConst::CommaSep %
                          ::AltitudeColumn % CsvConst::CommaSep %
                          ::PitchColumn % CsvConst::CommaSep %
                          ::BankColumn % CsvConst::CommaSep %
                          ::TrueHeadingColumn % CsvConst::CommaSep %
                          ::IndicatedAirspeedColumn % CsvConst::CommaSep %
                          ::OnGroundColumn % CsvConst::Ln
                          );

    bool ok = io.write(csv.toUtf8());
    Enumeration locationTypeEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::LocationType);
    Enumeration locationCategoryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::LocationCategory);
    Enumeration countryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::Country);
    if (ok) {
        for (const Location &location : locations) {
            QString title = location.title;
            QString description = location.description;
            QString identifier = location.identifier;
            const QString locationTypeSymbolicId = locationTypeEnumeration.getItemById(location.typeId).symbolicId;
            const QString locationCategorySymbolicId = locationCategoryEnumeration.getItemById(location.categoryId).symbolicId;
            const QString countrySymbolicId = countryEnumeration.getItemById(location.countryId).symbolicId;
            const QString csv = "\"" % title.replace("\"", "\"\"") % "\"" % CsvConst::CommaSep %
                                "\"" % description.replace("\"", "\"\"") % "\"" % CsvConst::CommaSep %
                                locationTypeSymbolicId % CsvConst::CommaSep %
                                locationCategorySymbolicId % CsvConst::CommaSep %
                                countrySymbolicId % CsvConst::CommaSep %
                                QString::number(location.attributes) % CsvConst::CommaSep %
                                "\"" % identifier.replace("\"", "\"\"") % "\"" % CsvConst::CommaSep %
                                Unit::formatCoordinate(location.latitude) % CsvConst::CommaSep %
                                Unit::formatCoordinate(location.longitude) % CsvConst::CommaSep %
                                Export::formatNumber(location.altitude) % CsvConst::CommaSep %
                                Export::formatNumber(location.pitch) % CsvConst::CommaSep %
                                Export::formatNumber(location.bank) % CsvConst::CommaSep %
                                Export::formatNumber(location.trueHeading) % CsvConst::CommaSep %
                                QString::number(location.indicatedAirspeed) % CsvConst::CommaSep %
                                (location.onGround ? "true" : "false") % CsvConst::Ln;
            ok = io.write(csv.toUtf8());

            if (!ok) {
                break;
            }
        }
    }
    return ok;
}

// PRIVATE
