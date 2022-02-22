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
#include <QWidget>

#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Flight/src/FlightAugmentation.h"
#include "GPXParser.h"
#include "GPXImportPlugin.h"

class GPXImportPluginPrivate
{
public:
    GPXImportPluginPrivate()
    {}

    QXmlStreamReader xml;
    QDateTime firstDateTimeUtc;
    QString flightNumber;
    QString title;

    static inline const QString FileExtension {QStringLiteral("gpx")};
};

// PUBLIC

GPXImportPlugin::GPXImportPlugin() noexcept
    : d(std::make_unique<GPXImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("GPXImportPlugin::GPXImportPlugin: PLUGIN LOADED");
#endif
}

GPXImportPlugin::~GPXImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("GPXImportPlugin::~GPXImportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

Settings::PluginSettings GPXImportPlugin::getSettings() const noexcept
{
    return Settings::PluginSettings();
}

Settings::KeysWithDefaults GPXImportPlugin::getKeyWithDefaults() const noexcept
{
    return Settings::KeysWithDefaults();
}

void GPXImportPlugin::setSettings(Settings::ValuesByKey valuesByKey) noexcept
{}

QString GPXImportPlugin::getFileFilter() const noexcept
{
    return tr("GPX Exchange Format (*.%1)").arg(GPXImportPluginPrivate::FileExtension);
}

std::unique_ptr<QWidget> GPXImportPlugin::createOptionWidget() const noexcept
{
    return nullptr;
}

bool GPXImportPlugin::readFile(QFile &file) noexcept
{
    d->xml.setDevice(&file);
    parseGPX();

    bool ok = !d->xml.hasError();
#ifdef DEBUG
    if (!ok) {
        qDebug("GPXImportPlugin::import: XML error: %s", qPrintable(d->xml.errorString()));
    }
#endif
    return ok;
}

FlightAugmentation::Procedures GPXImportPlugin::getProcedures() const noexcept
{
    return FlightAugmentation::Procedure::All;
}

FlightAugmentation::Aspects GPXImportPlugin::getAspects() const noexcept
{
    return FlightAugmentation::Aspect::All;
}

QDateTime GPXImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->firstDateTimeUtc;
}

QString GPXImportPlugin::getTitle() const noexcept
{
    return d->title;
}

void GPXImportPlugin::updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept
{}

void GPXImportPlugin::updateExtendedFlightInfo([[maybe_unused]] Flight &flight) noexcept
{}

void GPXImportPlugin::updateExtendedFlightCondition([[maybe_unused]] FlightCondition &flightCondition) noexcept
{}

// PROTECTED SLOTS

void GPXImportPlugin::onRestoreDefaultSettings() noexcept
{}

// PRIVATE

void GPXImportPlugin::parseGPX() noexcept
{
    std::unique_ptr<GPXParser> parser;
    parser = std::make_unique<GPXParser>(d->xml);

    // @todo IMPLEMENT ME!!!
    if (parser != nullptr) {
        parser->parse();
//        d->firstDateTimeUtc = parser->getFirstDateTimeUtc();
//        d->title = parser->getDocumentName();
        if (d->title.isEmpty()) {
            d->title = tr("GPX import");
        }
//        d->flightNumber = parser->getFlightNumber();
    }
}
