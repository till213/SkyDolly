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
#include <vector>
#include <unordered_set>

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QTimeZone>
#include <QFlags>
#include <QWidget>

#include <Kernel/Const.h>
#include <Kernel/Settings.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/AircraftService.h>
#include <Persistence/Migration.h>
#include "SdLogImportSettings.h"
#include "SdLogImportPlugin.h"

struct SdlogImportPluginPrivate
{
    Flight *flight {nullptr};
    SdLogImportSettings pluginSettings;

    static constexpr const char *FileExtension {Const::LogbookExtension};
};

// PUBLIC

SdlogImportPlugin::SdlogImportPlugin() noexcept
    : d(std::make_unique<SdlogImportPluginPrivate>())
{}

SdlogImportPlugin::~SdlogImportPlugin() = default;

// PROTECTED

FlightImportPluginBaseSettings &SdlogImportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString SdlogImportPlugin::getFileExtension() const noexcept
{
    return SdlogImportPluginPrivate::FileExtension;
}

QString SdlogImportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("GPX exchange format (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> SdlogImportPlugin::createOptionWidget() const noexcept
{
    return nullptr;
}

bool SdlogImportPlugin::importFlight(QFile &file, Flight &flight) noexcept
{
    bool ok {true};
    d->flight = &flight;

    return ok;
}

FlightAugmentation::Procedures SdlogImportPlugin::getProcedures() const noexcept
{
    return FlightAugmentation::Procedure::All;
}

FlightAugmentation::Aspects SdlogImportPlugin::getAspects() const noexcept
{
    return FlightAugmentation::Aspect::All;
}

QDateTime SdlogImportPlugin::getStartDateTimeUtc() noexcept
{
    // TODO IMPLEMENT ME
    return {};
}

QString SdlogImportPlugin::getTitle() const noexcept
{
    QString title;
    // TODO IMPLEMENT ME
    return title;
}

void SdlogImportPlugin::updateExtendedAircraftInfo([[maybe_unused]] AircraftInfo &aircraftInfo) noexcept
{}

void SdlogImportPlugin::updateExtendedFlightInfo([[maybe_unused]] Flight &flight) noexcept
{}

void SdlogImportPlugin::updateExtendedFlightCondition([[maybe_unused]] FlightCondition &flightCondition) noexcept
{}

// PRIVATE
