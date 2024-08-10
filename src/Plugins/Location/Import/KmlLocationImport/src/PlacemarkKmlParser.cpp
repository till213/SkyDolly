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
#include <QString>
#include <QStringLiteral>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Const.h>
#include <Kernel/Convert.h>
#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/Service/EnumerationService.h>
#include <Persistence/PersistedEnumerationItem.h>
#include "Kml.h"
#include "KmlLocationImportSettings.h"
#include "KmlParserIntf.h"
#include "PlacemarkKmlParser.h"

struct PlacemarkKmlParserPrivate
{
    PlacemarkKmlParserPrivate(const KmlLocationImportSettings &pluginSettings)
        : pluginSettings {pluginSettings}
    {}

    EnumerationService enumerationService;
    Enumeration categoryEnumeration {enumerationService.getEnumerationByName(EnumerationService::LocationCategory)};
    const KmlLocationImportSettings &pluginSettings;

    const std::int64_t ImportTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeImportSymId).id()};
    const std::int64_t KeepEngineEventId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventKeepSymId).id()};
    const std::int64_t WorldId {PersistedEnumerationItem(EnumerationService::Country, EnumerationService::CountryWorldSymId).id()};
    const std::int64_t OtherCategoryId {PersistedEnumerationItem(EnumerationService::LocationCategory, "OT").id()};
    const std::int64_t AirportCategoryId {PersistedEnumerationItem(EnumerationService::LocationCategory, "AP").id()};
    const std::int64_t CityCategoryId {PersistedEnumerationItem(EnumerationService::LocationCategory, "CI").id()};
    const std::int64_t PointOfInterestCategoryId {PersistedEnumerationItem(EnumerationService::LocationCategory, "PO").id()};

    std::int64_t currentCategoryId {OtherCategoryId};
};

// PUBLIC

PlacemarkKmlParser::PlacemarkKmlParser(const KmlLocationImportSettings &pluginSettings) noexcept
    : AbstractKmlParser(),
      d {std::make_unique<PlacemarkKmlParserPrivate>(pluginSettings)}
{}

PlacemarkKmlParser::~PlacemarkKmlParser() = default;

// Placemark KML files (are expected to) have at least one Placemark element
std::vector<Location> PlacemarkKmlParser::parse(QXmlStreamReader &xmlStreamReader) noexcept
{
    initialise(&xmlStreamReader);
    std::vector<Location> locations = parseKML();
    return locations;
}

// PROTECTED

void PlacemarkKmlParser::parseFolderName(const QString &folderName) noexcept
{
    guesstimateCurrentCategoryId(folderName);
}

void PlacemarkKmlParser::parsePlacemark(std::vector<Location> &locations) noexcept
{
    auto *xml = getXmlStreamReader();
    Location location;
    location.typeId = d->ImportTypeId;
    location.engineEventId = d->KeepEngineEventId;
    location.countryId = d->pluginSettings.getDefaultCountryId();
    location.indicatedAirspeed = d->pluginSettings.getDefaultIndicatedAirspeed();
    location.categoryId = d->currentCategoryId;

    while (xml->readNextStartElement()) {
        const QStringView xmlName = xml->name();
#ifdef DEBUG
        qDebug() << "PlacemarkKmlParser::parsePlacemark: XML start element:" << xmlName.toString();
#endif        
        if (xmlName == Kml::name) {
            location.title = xml->readElementText();
        } else if (xmlName == Kml::description) {
            location.description = xml->readElementText();
            unHtmlify(location.description);
        } else if (xmlName == Kml::Point) {
            parsePoint(location);
        } else {
            xml->skipCurrentElement();
        }
    }

    if (location.categoryId == d->AirportCategoryId) {
        const auto icao = extractIcao(location.description);
        location.identifier = icao;
    }

    locations.push_back(std::move(location));
}

// PRIVATE

void PlacemarkKmlParser::parsePoint(Location &location) noexcept
{
    bool ok {true};
    auto *xml = getXmlStreamReader();
    while (xml->readNextStartElement()) {
        const QStringView xmlName = xml->name();
#ifdef DEBUG
        qDebug() << "PlacemarkKmlParser::parsePoint: XML start element:" << xmlName.toString();
#endif
        if (xmlName == Kml::coordinates) {
            const QString coordinatesText = xml->readElementText();
            const QStringList coordinates = coordinatesText.split(",");
            if (coordinates.count() == 3) {

                const double longitude = coordinates.at(0).toDouble(&ok);
                if (!ok) {
                    xml->raiseError("Invalid longitude number.");
                }
                const double latitude = coordinates.at(1).toDouble(&ok);
                if (!ok) {
                    xml->raiseError("Invalid latitude number.");
                }
                const double altitude = coordinates.at(2).toDouble(&ok);
                if (!ok) {
                    xml->raiseError("Invalid altitude number.");
                }
                if (ok) {
                    location.latitude = latitude;
                    location.longitude = longitude;
                    if (!qFuzzyIsNull(altitude)) {
                        location.altitude = Convert::metersToFeet(altitude);
                    } else {
                        // TODO Provide an option to:
                        // - place aircraft on ground
                        // - use default altitude
                        // - import "as is" (0.0 feet)
                        location.altitude = d->pluginSettings.getDefaultAltitude();
                    }
                }

            } else {
                xml->raiseError("Invalid GPS coordinate.");
            }
        } else {
            xml->skipCurrentElement();
        }
    }
}

void PlacemarkKmlParser::guesstimateCurrentCategoryId(const QString &folderName) noexcept
{
    const auto lower = folderName.toLower();
    if (lower.contains("airport")) {
        d->currentCategoryId = d->AirportCategoryId;
    } else if (lower.contains("points of interest") || lower.contains("poi")) {
        d->currentCategoryId = d->PointOfInterestCategoryId;
    } else if (lower.contains("cities") || lower.contains("city")) {
        d->currentCategoryId = d->CityCategoryId;
    } else {
        d->currentCategoryId = d->OtherCategoryId;
    }
}

void PlacemarkKmlParser::unHtmlify(QString &description) noexcept
{
    static const QRegularExpression imageRegExp {R"(<img([\w\W]+?)\/>)"};
    description.replace("<br>", "\n");
    description.replace("<b>", "").replace("</b>", "");
    description.replace("<i>", "").replace("</i>", "");
    description.replace(imageRegExp, "");
    description = description.trimmed();
}

QString PlacemarkKmlParser::extractIcao(const QString &description)
{
    static const QRegularExpression icaoRegExp {R"(([A-Z]{4}))"};
    QString icao {description};
    icao.replace("ICAO:", "");
    auto match = icaoRegExp.match(icao);
    if (match.hasMatch()) {
        icao = match.captured(1);
    }
    return icao;
}
