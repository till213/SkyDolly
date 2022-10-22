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

#include <QIODevice>
#include <QTextStream>
#include <QTextCodec>
#include <QRegularExpression>

#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/Service/EnumerationService.h>
#include "LittleNavmapCsvParser.h"

namespace
{
    // Also refer to Locations.csv
    constexpr int TitleIndex = 1;
    constexpr int DescriptionIndex = 2;
    constexpr int CategoryIndex = 3;
    constexpr int CountryIndex = 4;
    constexpr int IdentifierIndex = 5;
    constexpr int LatitudeIndex = 6;
    constexpr int LongitudeIndex = 7;
    constexpr int AltitudeIndex = 8;
    constexpr int PitchIndex = 9;
    constexpr int BankIndex = 10;
    constexpr int TrueHeadingIndex = 11;
    constexpr int IndicatedAirspeedIndex = 12;
    constexpr int OnGroundIndex = 13;
    constexpr int AttributesIndex = 14;
}

struct LittleNavmapCsvParserPrivate
{
    EnumerationService enumerationService;
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
    static const QRegularExpression csvRegExp {
        R"(["|,]?([^"]+)["]?,)"       // Title
        R"(["]?([^"]*)["]?,)"         // Description (optional)
        R"(["]?([^"]*)["]?,)"         // Category (symbolic ID) (optional)
        R"(["]?([^"]*)["]?,)"         // Country (symbolic ID) (optional)
        R"(["]?([^"]*)["]?,)"         // Identifier (optional)
        R"(([+-]?[0-9]*[.]?[0-9]+),)" // Latitude
        R"(([+-]?[0-9]*[.]?[0-9]+),)" // Longitude
        R"(([+-]?[0-9]*[.]?[0-9]+),)" // Altitude (optional)
        R"(([+-]?[0-9]*[.]?[0-9]+),)" // Pitch (optional)
        R"(([+-]?[0-9]*[.]?[0-9]+),)" // Bank (optional)
        R"(([+-]?[0-9]*[.]?[0-9]+),)" // True Heading (optional)
        R"(([+-]?[\d]*),)"            // Indicated airspeed (optional)
        R"(["]?(false|true)*["]?,)"   // On Ground (optional)
        R"(([+-]?[\d]*)$)"            // Attributes (optional)
    };
    std::vector<Location> locations;

    QString csv = textStream.readLine();
    if (csv.startsWith("\"Type\"") || csv.startsWith("Type")) {
        // Skip column names
        csv = textStream.readLine();
    }

    bool success {true};
    while (success && !csv.isNull()) {
        QRegularExpressionMatch match = csvRegExp.match(csv);
        if (match.hasMatch()) {
            Location location = parseLocation(match, success);

        }
        csv = textStream.readLine();
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return locations;
}

Location LittleNavmapCsvParser::parseLocation(QRegularExpressionMatch locationMatch, bool &ok) noexcept
{
    ok = true;
    Location location;
    location.title = locationMatch.captured(::TitleIndex);
    location.description = locationMatch.captured(::DescriptionIndex).replace("\\n", "\n");
    Enumeration locationType = d->enumerationService.getEnumerationByName(EnumerationService::LocationType, &ok);
    if (ok) {
        location.typeId = locationType.getItemBySymbolicId(EnumerationService::LocationTypeImportSymbolicId).id;
    }
    Enumeration locationCategory = d->enumerationService.getEnumerationByName(EnumerationService::LocationCategory, &ok);
    if (ok) {
        const QString categorySymbolicId = locationMatch.captured(::CategoryIndex);
        location.categoryId = locationCategory.getItemBySymbolicId(categorySymbolicId).id;
    }
    Enumeration country = d->enumerationService.getEnumerationByName(EnumerationService::Country, &ok);
    if (ok) {
        const QString countrySymbolicId = locationMatch.captured(::CountryIndex);
        location.countryId = country.getItemBySymbolicId(countrySymbolicId).id;
    }
    if (ok) {
        location.identifier = locationMatch.captured(::IdentifierIndex);
    }
    if (ok) {
        location.latitude = locationMatch.captured(::LatitudeIndex).toDouble(&ok);
    }
    if (ok) {
        location.longitude = locationMatch.captured(::LongitudeIndex).toDouble(&ok);
    }
    if (ok) {
        location.altitude = locationMatch.captured(::AltitudeIndex).toDouble(&ok);
    }
    if (ok) {
        location.pitch = locationMatch.captured(::PitchIndex).toDouble(&ok);
    }
    if (ok) {
        location.bank = locationMatch.captured(::BankIndex).toDouble(&ok);
    }
    if (ok) {
        location.trueHeading = locationMatch.captured(::TrueHeadingIndex).toDouble(&ok);
    }
    if (ok) {
        location.indicatedAirspeed = locationMatch.captured(::IndicatedAirspeedIndex).toInt(&ok);
    }
    if (ok) {
        location.attributes = locationMatch.captured(::AttributesIndex).toLongLong(&ok);
    }
    if (ok) {
        location.onGround = locationMatch.captured(::OnGroundIndex).toLower() == "true" ? true : false;
    }

    return location;
}
