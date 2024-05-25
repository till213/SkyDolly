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

#include <QFile>
#include <QFileInfo>

#include <Kernel/Unit.h>
#include <Model/Flight.h>
#include <Model/FlightData.h>
#include <Model/AircraftInfo.h>
#include <Flight/FlightAugmentation.h>
#include "CsvParserIntf.h"
#include "FlightRadar24CsvParser.h"
#include "FlightRecorderCsvParser.h"
#include "CsvImportSettings.h"
#include "CsvImportOptionWidget.h"
#include "CsvImportPlugin.h"

struct CsvImportPluginPrivate
{
    CsvImportSettings pluginSettings;
    static inline const QString FileExtension {"csv"};
};

// PUBLIC

CsvImportPlugin::CsvImportPlugin() noexcept
    : d {std::make_unique<CsvImportPluginPrivate>()}
{}

CsvImportPlugin::~CsvImportPlugin() = default;

std::vector<FlightData> CsvImportPlugin::importFlightData(QIODevice &io, bool &ok) noexcept
{
    std::vector<FlightData> flights;
    std::unique_ptr<CsvParserIntf> parser;
    switch (d->pluginSettings.getFormat()) {
    case CsvImportSettings::Format::Flightradar24:
        parser = std::make_unique<FlightRadar24CsvParser>();
        break;
    case CsvImportSettings::Format::FlightRecorder:
        parser = std::make_unique<FlightRecorderCsvParser>();
        break;
    }
    ok = false;
    if (parser != nullptr) {
        FlightData flightData = parser->parse(io, ok);
        if (ok) {
            ok = flightData.hasRecording();
        }
        if (ok) {
            enrichFlightData(flightData);
            flights.push_back(std::move(flightData));
        }
    }
    return flights;
}

// PROTECTED

FlightImportPluginBaseSettings &CsvImportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString CsvImportPlugin::getFileExtension() const noexcept
{
    return CsvImportPluginPrivate::FileExtension;
}

QString CsvImportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Comma-separated values (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> CsvImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<CsvImportOptionWidget>(d->pluginSettings);
}

FlightAugmentation::Procedures CsvImportPlugin::getAugmentationProcedures() const noexcept
{
    FlightAugmentation::Procedures procedures;
    switch (d->pluginSettings.getFormat()) {
    case CsvImportSettings::Format::Flightradar24:
        // Do not augment heading and velocity
        procedures = FlightAugmentation::Procedure::All;
        break;
    case CsvImportSettings::Format::FlightRecorder:
        procedures = FlightAugmentation::Procedure::None;
        break;
    }

    return procedures;
}

FlightAugmentation::Aspects CsvImportPlugin::getAugmentationAspects() const noexcept
{
    FlightAugmentation::Aspects aspects;
    switch (d->pluginSettings.getFormat()) {
    case CsvImportSettings::Format::Flightradar24:
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


// PRIVATE

void CsvImportPlugin::enrichFlightData(FlightData &flightData) const noexcept
{
    flightData.title = generateTitle();
}

QString CsvImportPlugin::generateTitle() const noexcept
{
    QString title;
    switch (d->pluginSettings.getFormat()) {
    case CsvImportSettings::Format::Flightradar24:
        // Do not augment heading and velocity
        title = QObject::tr("Flightradar24 CSV import");
        break;
    case CsvImportSettings::Format::FlightRecorder:
        title = QObject::tr("Flight Recorder CSV import");
        break;
    }
    return title;
}
