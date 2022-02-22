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

#include <QFile>
#include <QFileInfo>
#include <QFlags>

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Flight/src/FlightAugmentation.h"
#include "CSVParserIntf.h"
#include "SkyDollyCSVParser.h"
#include "FlightRadar24CSVParser.h"
#include "FlightRecorderCSVParser.h"
#include "CSVImportSettings.h"
#include "CSVImportOptionWidget.h"
#include "CSVImportPlugin.h"

class CSVImportPluginPrivate
{
public:
    CSVImportPluginPrivate()
    {}

    CSVImportSettings importSettings;
    QDateTime firstDateTimeUtc;
    QString flightNumber;

    static inline const QString FileExtension {QStringLiteral("csv")};
};

// PUBLIC

CSVImportPlugin::CSVImportPlugin() noexcept
    : d(std::make_unique<CSVImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("CSVImportPlugin::CSVImportPlugin: PLUGIN LOADED");
#endif
}

CSVImportPlugin::~CSVImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("CSVImportPlugin::~CSVImportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

Settings::PluginSettings CSVImportPlugin::getSettings() const noexcept
{
    return d->importSettings.getSettings();
}

Settings::KeysWithDefaults CSVImportPlugin::getKeyWithDefaults() const noexcept
{
    return d->importSettings.getKeysWithDefault();
}

void CSVImportPlugin::setSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    d->importSettings.setSettings(valuesByKey);
}

QString CSVImportPlugin::getFileFilter() const noexcept
{
    return tr("Comma-separated values (*.%1)").arg(CSVImportPluginPrivate::FileExtension);
}

std::unique_ptr<QWidget> CSVImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<CSVImportOptionWidget>(d->importSettings);
}

bool CSVImportPlugin::readFile(QFile &file) noexcept
{
    bool ok;

    std::unique_ptr<CSVParserIntf> parser;
    switch (d->importSettings.m_format) {
    case CSVImportSettings::Format::SkyDolly:
        parser = std::make_unique<SkyDollyCSVParser>();
        break;
    case CSVImportSettings::Format::FlightRadar24:
        parser = std::make_unique<FlightRadar24CSVParser>();
        break;
    case CSVImportSettings::Format::FlightRecorder:
        parser = std::make_unique<FlightRecorderCSVParser>();
        break;
    default:
        parser = nullptr;
        ok = false;
        break;
    }
    if (parser != nullptr) {
        ok = parser->parse(file, d->firstDateTimeUtc, d->flightNumber);
    }
    return ok;
}

FlightAugmentation::Procedures CSVImportPlugin::getProcedures() const noexcept
{
    return FlightAugmentation::Procedure::None;
}

FlightAugmentation::Aspects CSVImportPlugin::getAspects() const noexcept
{
    FlightAugmentation::Aspects aspects;
    switch (d->importSettings.m_format)
    {
    case CSVImportSettings::Format::SkyDolly:
        aspects = FlightAugmentation::Aspect::None;
        break;
    case CSVImportSettings::Format::FlightRadar24:
        // Do not augment heading and velocity
        aspects = FlightAugmentation::Aspect::All;
        aspects.setFlag(FlightAugmentation::Aspect::Heading, false);
        aspects.setFlag(FlightAugmentation::Aspect::Velocity, false);
        break;
    case CSVImportSettings::Format::FlightRecorder:
        aspects = FlightAugmentation::Aspect::None;
        break;
    default:
        aspects = FlightAugmentation::Aspect::All;
        break;
    }

    return aspects;
}

QDateTime CSVImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->firstDateTimeUtc;
}

QString CSVImportPlugin::getTitle() const noexcept
{
    QString title;
    switch (d->importSettings.m_format)
    {
    case CSVImportSettings::Format::SkyDolly:
        title = tr("Sky Dolly CSV import");
        break;
    case CSVImportSettings::Format::FlightRadar24:
        // Do not augment heading and velocity
        title = tr("Flightradar24 CSV import");
        break;
    case CSVImportSettings::Format::FlightRecorder:
        title = tr("Flight Recorder CSV import");
        break;
    default:
        title = tr("CSV import");
        break;
    }
    return title;
}

void CSVImportPlugin::updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept
{
    aircraftInfo.flightNumber = d->flightNumber;
}

void CSVImportPlugin::updateExtendedFlightInfo(Flight &flight) noexcept
{}

void CSVImportPlugin::updateExtendedFlightCondition(FlightCondition &flightCondition) noexcept
{}

// PROTECTED SLOTS

void CSVImportPlugin::onRestoreDefaultSettings() noexcept
{
    d->importSettings.restoreDefaults();
}
