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

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Flight/src/FlightAugmentation.h"
#include "CSVParserIntf.h"
#include "SkyDollyCSVParser.h"
#include "FlightRadar24CSVParser.h"
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
    return FlightAugmentation::Procedures::None;
}

FlightAugmentation::Aspects CSVImportPlugin::getAspects() const noexcept
{
    FlightAugmentation::Aspects aspects;
    switch (d->importSettings.m_format)
    {
    case CSVImportSettings::Format::SkyDolly:
        aspects = FlightAugmentation::Aspects::None;
        break;
    case CSVImportSettings::Format::FlightRadar24:
        // Do not augment heading and velocity
        aspects = FlightAugmentation::Aspects::Pitch | FlightAugmentation::Aspects::Bank | FlightAugmentation::Aspects::Engine | FlightAugmentation::Aspects::Light;
        break;
    default:
        aspects = FlightAugmentation::Aspects::All;
        break;
    }

    return aspects;
}

QDateTime CSVImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->firstDateTimeUtc;
}

void CSVImportPlugin::updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept
{
    aircraftInfo.flightNumber = d->flightNumber;
}

void CSVImportPlugin::updateFlight(const QFile &file) noexcept
{
    Unit unit;
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    flight.setTitle(tr("CSV import"));

    const QString description = tr("Aircraft imported on %1 from file: %2").arg(unit.formatDateTime(QDateTime::currentDateTime()), file.fileName());
    flight.setDescription(description);
    flight.setCreationDate(QFileInfo(file).birthTime());
}

// PROTECTED SLOTS

void CSVImportPlugin::onRestoreDefaultSettings() noexcept
{
    d->importSettings.restoreDefaults();
}
