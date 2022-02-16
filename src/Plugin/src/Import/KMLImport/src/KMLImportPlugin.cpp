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
#include <tuple>
#include <vector>

#include <QFile>
#include <QFileInfo>
#include <QStringLiteral>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QTimeZone>

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "KMLImportSettings.h"
#include "KMLParserIntf.h"
#include "FlightAwareKMLParser.h"
#include "FlightRadar24KMLParser.h"
#include "KMLImportPlugin.h"

class KMLImportPluginPrivate
{
public:
    KMLImportPluginPrivate()
    {}

    QXmlStreamReader xml;
    Unit unit;
    KMLImportSettings importSettings;
    QDateTime firstDateTimeUtc;
    QDateTime lastDateTimeUtc;
    QString flightNumber;
    QString title;
};

// PUBLIC

KMLImportPlugin::KMLImportPlugin() noexcept
    : d(std::make_unique<KMLImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("KMLImportPlugin::KMLImportPlugin: PLUGIN LOADED");
#endif
}

KMLImportPlugin::~KMLImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("KMLImportPlugin::~KMLImportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

bool KMLImportPlugin::readFile(QFile &file) noexcept
{
    d->xml.setDevice(&file);
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::import: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == QStringLiteral("kml")) {
            parseKML();
        } else {
            d->xml.raiseError(QStringLiteral("The file is not a KML file."));
        }
    }

    bool ok = !d->xml.hasError();
#ifdef DEBUG
    if (!ok) {
        qDebug("KMLImportPlugin::import: XML error: %s", qPrintable(d->xml.errorString()));
    }
#endif
    return ok;
}

QDateTime KMLImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->firstDateTimeUtc;
}

void KMLImportPlugin::updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept
{
    aircraftInfo.flightNumber = d->flightNumber;
}

void KMLImportPlugin::updateFlight(const QFile &file) noexcept
{
    updateFlightInfo(file);
    updateFlightCondition();
}

// PROTECTED

Settings::PluginSettings KMLImportPlugin::getSettings() const noexcept
{
    return d->importSettings.getSettings();
}

Settings::KeysWithDefaults KMLImportPlugin::getKeyWithDefaults() const noexcept
{
    return d->importSettings.getKeysWithDefault();
}

void KMLImportPlugin::setSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    d->importSettings.setSettings(valuesByKey);
}

// PRIVATE

void KMLImportPlugin::parseKML() noexcept
{
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::readKML: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == QStringLiteral("Document")) {
            parseName();
            parseDocument();
        } else {
            d->xml.raiseError(QStringLiteral("The file is not a KML document."));
        }
    } else {
        d->xml.raiseError(QStringLiteral("Error reading the XML data."));
    }
}

void KMLImportPlugin::parseName() noexcept
{
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::readDocument: XML start element: %s", qPrintable(d->xml.name().toString()));
#endif
        if (d->xml.name() == QStringLiteral("name")) {
            d->title = d->xml.readElementText();
        } else {
            d->xml.raiseError(QStringLiteral("The KML document does not have a name element."));
        }
    }
}

void KMLImportPlugin::parseDocument() noexcept
{
    std::unique_ptr<KMLParserIntf> parser;
    switch (d->importSettings.format) {
    case KMLImportSettings::Format::FlightAware:
        parser = std::make_unique<FlightAwareKMLParser>(d->xml);
        break;
    case KMLImportSettings::Format::FlightRadar24:
        parser = std::make_unique<FlightRadar24KMLParser>(d->xml);
        break;
    default:
        break;
    }
    if (parser != nullptr) {
        parser->parse(d->firstDateTimeUtc, d->lastDateTimeUtc, d->flightNumber);
    }
}

void KMLImportPlugin::updateFlightInfo(const QFile &file) noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    flight.setTitle(d->title);

    const QString description = tr("Aircraft imported on %1 from file: %2").arg(d->unit.formatDateTime(QDateTime::currentDateTime()), file.fileName());
    flight.setDescription(description);
    flight.setCreationDate(QFileInfo(file).birthTime());
}

void KMLImportPlugin::updateFlightCondition() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    FlightCondition flightCondition;

    flightCondition.startLocalTime = d->firstDateTimeUtc.toLocalTime();
    flightCondition.startZuluTime = d->firstDateTimeUtc;
    flightCondition.endLocalTime = d->lastDateTimeUtc.toLocalTime();
    flightCondition.endZuluTime = d->lastDateTimeUtc;

    flight.setFlightCondition(flightCondition);
}
