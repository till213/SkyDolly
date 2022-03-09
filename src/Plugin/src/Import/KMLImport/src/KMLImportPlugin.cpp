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
#include <QFlags>

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Flight/src/FlightAugmentation.h"
#include "KMLImportSettings.h"
#include "KMLParserIntf.h"
#include "FlightAwareKMLParser.h"
#include "FlightRadar24KMLParser.h"
#include "GenericKMLParser.h"
#include "KMLImportOptionWidget.h"
#include "KMLImportPlugin.h"

class KMLImportPluginPrivate
{
public:
    KMLImportPluginPrivate()
    {}

    QXmlStreamReader xml;
    KMLImportSettings settings;
    QDateTime firstDateTimeUtc;
    QString flightNumber;
    QString title;

    static inline const QString FileExtension {QStringLiteral("kml")};
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

void KMLImportPlugin::addSettings(Settings::PluginSettings &settings) const noexcept
{
    d->settings.addSettings(settings);
}

void KMLImportPlugin::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    d->settings.addKeysWithDefaults(keysWithDefaults);
}

void KMLImportPlugin::restoreSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    d->settings.applySettings(valuesByKey);
}

QString KMLImportPlugin::getFileFilter() const noexcept
{
    return tr("Keyhole markup language (*.%1)").arg(KMLImportPluginPrivate::FileExtension);
}

std::unique_ptr<QWidget> KMLImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<KMLImportOptionWidget>(d->settings);
}

bool KMLImportPlugin::readFile(QFile &file) noexcept
{
    d->xml.setDevice(&file);
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug("KMLImportPlugin::readFile: XML start element: %s", qPrintable(d->xml.name().toString()));
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
        qDebug("KMLImportPlugin::readFile: XML error: %s", qPrintable(d->xml.errorString()));
    }
#endif
    return ok;
}

FlightAugmentation::Procedures KMLImportPlugin::getProcedures() const noexcept
{
    return FlightAugmentation::Procedure::All;
}

FlightAugmentation::Aspects KMLImportPlugin::getAspects() const noexcept
{
    FlightAugmentation::Aspects aspects;
    switch (d->settings.m_format)
    {
    case KMLImportSettings::Format::FlightAware:
        aspects = FlightAugmentation::Aspect::All;
        break;
    case KMLImportSettings::Format::FlightRadar24:
        // Do not augment heading and velocity
        aspects = FlightAugmentation::Aspect::All;
        aspects.setFlag(FlightAugmentation::Aspect::Heading, false);
        aspects.setFlag(FlightAugmentation::Aspect::Velocity, false);
        return aspects;
        break;
    default:
        aspects = FlightAugmentation::Aspect::All;
        break;
    }

    return aspects;
}

QDateTime KMLImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->firstDateTimeUtc;
}

QString KMLImportPlugin::getTitle() const noexcept
{
    return d->title;
}

void KMLImportPlugin::updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept
{
    aircraftInfo.flightNumber = d->flightNumber;
}

void KMLImportPlugin::updateExtendedFlightInfo([[maybe_unused]] Flight &flight) noexcept
{}

void KMLImportPlugin::updateExtendedFlightCondition([[maybe_unused]] FlightCondition &flightCondition) noexcept
{}

// PROTECTED SLOTS

void KMLImportPlugin::onRestoreDefaultSettings() noexcept
{
    d->settings.restoreDefaults();
}

// PRIVATE

void KMLImportPlugin::parseKML() noexcept
{
    std::unique_ptr<KMLParserIntf> parser;
    switch (d->settings.m_format) {
    case KMLImportSettings::Format::FlightAware:
        parser = std::make_unique<FlightAwareKMLParser>(d->xml);
        break;
    case KMLImportSettings::Format::FlightRadar24:
        parser = std::make_unique<FlightRadar24KMLParser>(d->xml);
        break;
    case KMLImportSettings::Format::Generic:
        parser = std::make_unique<GenericKMLParser>(d->xml);
        break;
    default:
        parser = nullptr;
        break;
    }
    if (parser != nullptr) {
        parser->parse();
        d->firstDateTimeUtc = parser->getFirstDateTimeUtc();
        d->title = parser->getDocumentName();
        if (d->title.isEmpty()) {
            d->title = tr("KML import");
        }
        d->flightNumber = parser->getFlightNumber();
    }
}
