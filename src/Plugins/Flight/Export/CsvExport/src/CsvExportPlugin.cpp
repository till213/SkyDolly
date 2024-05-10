/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation ios (the "Software"), to deal in the Software
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
#include <QWidget>
#include <QFile>
// Implements the % operator for string concatenation
#include <QStringBuilder>
#include <QString>

#include <Model/Flight.h>
#include <Model/FlightData.h>
#include <Model/Aircraft.h>
#include "CsvExportSettings.h"
#include "CsvExportOptionWidget.h"
#include "CsvWriterIntf.h"
#include "FlightRadar24CsvWriter.h"
#include "PositionAndAttitudeCsvWriter.h"
#include "CsvExportPlugin.h"

struct CsvExportPluginPrivate
{
    CsvExportSettings pluginSettings;

    static inline const QString FileExtension {"csv"};
};

// PUBLIC

CsvExportPlugin::CsvExportPlugin() noexcept
    : d(std::make_unique<CsvExportPluginPrivate>())
{}

CsvExportPlugin::~CsvExportPlugin() = default;

// PROTECTED

FlightExportPluginBaseSettings &CsvExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString CsvExportPlugin::getFileExtension() const noexcept
{
    return CsvExportPluginPrivate::FileExtension;
}

QString CsvExportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Comma-separated values (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> CsvExportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<CsvExportOptionWidget>(d->pluginSettings);
}

bool CsvExportPlugin::exportFlightData([[maybe_unused]] const FlightData &flightData, [[maybe_unused]] QIODevice &io) const  noexcept
{
    // No multi aircraft support
    return false;
}

bool CsvExportPlugin::exportAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const  noexcept
{
    std::unique_ptr<CsvWriterIntf> writer;
    switch (d->pluginSettings.getFormat()) {
    case CsvExportSettings::Format::Flightradar24:
        writer = std::make_unique<FlightRadar24CsvWriter>(d->pluginSettings);
        break;
    case CsvExportSettings::Format::PositionAndAttitude:
        writer = std::make_unique<PositionAndAttitudeCsvWriter>(d->pluginSettings);
        break;
    }

    bool ok {false};
    if (writer != nullptr) {
        io.setTextModeEnabled(true);
        ok = writer->write(flightData, aircraft, io);
    }

    return ok;
}
