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
#include <cstdint>

#include <QFile>
#include <QFileInfo>
#include <QFlags>

#include <Kernel/Unit.h>
#include <Model/AircraftInfo.h>
#include <Flight/FlightAugmentation.h>
#include "CsvParserIntf.h"
#include "SkyDollyCsvParser.h"
#include "FlightRadar24CsvParser.h"
#include "FlightRecorderCsvParser.h"
#include "CsvImportSettings.h"
#include "CsvImportOptionWidget.h"
#include "CsvImportPlugin.h"

struct CsvImportPluginPrivate
{
    CsvImportPluginPrivate()
    {}

    CsvImportSettings pluginSettings;
    QDateTime firstDateTimeUtc;
    QString flightNumber;

    static inline const QString FileSuffix {QStringLiteral("csv")};
};

// PUBLIC

CsvImportPlugin::CsvImportPlugin() noexcept
    : d(std::make_unique<CsvImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug() << "CsvImportPlugin::CsvImportPlugin: PLUGIN LOADED";
#endif
}

CsvImportPlugin::~CsvImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug() << "CsvImportPlugin::~CsvImportPlugin: PLUGIN UNLOADED";
#endif
}

// PROTECTED

ImportPluginBaseSettings &CsvImportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString CsvImportPlugin::getFileSuffix() const noexcept
{
    return CsvImportPluginPrivate::FileSuffix;
}

QString CsvImportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Comma-separated values (*.%1)").arg(getFileSuffix());
}

std::unique_ptr<QWidget> CsvImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<CsvImportOptionWidget>(d->pluginSettings);
}

bool CsvImportPlugin::importFlight(QFile &file, Flight &flight) noexcept
{
    bool ok;

    std::unique_ptr<CsvParserIntf> parser;
    switch (d->pluginSettings.getFormat()) {
    case CsvImportSettings::Format::SkyDolly:
        parser = std::make_unique<SkyDollyCsvParser>();
        break;
    case CsvImportSettings::Format::FlightRadar24:
        parser = std::make_unique<FlightRadar24CsvParser>();
        break;
    case CsvImportSettings::Format::FlightRecorder:
        parser = std::make_unique<FlightRecorderCsvParser>();
        break;
    }
    if (parser != nullptr) {
        ok = parser->parse(file, d->firstDateTimeUtc, d->flightNumber, flight);
    } else {
        ok = false;
    }
    return ok;
}

FlightAugmentation::Procedures CsvImportPlugin::getProcedures() const noexcept
{
    FlightAugmentation::Procedures procedures;
    switch (d->pluginSettings.getFormat()) {
    case CsvImportSettings::Format::SkyDolly:
        procedures = FlightAugmentation::Procedure::None;
        break;
    case CsvImportSettings::Format::FlightRadar24:
        // Do not augment heading and velocity
        procedures = FlightAugmentation::Procedure::All;
        break;
    case CsvImportSettings::Format::FlightRecorder:
        procedures = FlightAugmentation::Procedure::None;
        break;
    }

    return procedures;
}

FlightAugmentation::Aspects CsvImportPlugin::getAspects() const noexcept
{
    FlightAugmentation::Aspects aspects;
    switch (d->pluginSettings.getFormat()) {
    case CsvImportSettings::Format::SkyDolly:
        aspects = FlightAugmentation::Aspect::None;
        break;
    case CsvImportSettings::Format::FlightRadar24:
        // Do not augment heading and velocity
        aspects = FlightAugmentation::Aspect::All;
        aspects.setFlag(FlightAugmentation::Aspect::Heading, false);
        aspects.setFlag(FlightAugmentation::Aspect::Velocity, false);
        break;
    case CsvImportSettings::Format::FlightRecorder:
        aspects = FlightAugmentation::Aspect::None;
        break;
    }

    return aspects;
}

QDateTime CsvImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->firstDateTimeUtc;
}

QString CsvImportPlugin::getTitle() const noexcept
{
    QString title;
    switch (d->pluginSettings.getFormat()) {
    case CsvImportSettings::Format::SkyDolly:
        title = QObject::tr("Sky Dolly CSV import");
        break;
    case CsvImportSettings::Format::FlightRadar24:
        // Do not augment heading and velocity
        title = QObject::tr("Flightradar24 CSV import");
        break;
    case CsvImportSettings::Format::FlightRecorder:
        title = QObject::tr("Flight Recorder CSV import");
        break;
    }
    return title;
}

void CsvImportPlugin::updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept
{
    aircraftInfo.flightNumber = d->flightNumber;
}

void CsvImportPlugin::updateExtendedFlightInfo([[maybe_unused]]Flight &flight) noexcept
{}

void CsvImportPlugin::updateExtendedFlightCondition([[maybe_unused]]FlightCondition &flightCondition) noexcept
{}
