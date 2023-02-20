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

#include <Kernel/Settings.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/AircraftService.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "LogbookWidget.h"
#include "LogbookSettings.h"
#include "LogbookPlugin.h"

struct LogbookPluginPrivate
{
    LogbookSettings moduleSettings;
    std::unique_ptr<AircraftService> aircraftService {std::make_unique<AircraftService>()};
    std::unique_ptr<LogbookWidget> logbookWidget {std::make_unique<LogbookWidget>(moduleSettings)};
};

// PUBLIC

LogbookPlugin::LogbookPlugin(QObject *parent) noexcept
    : AbstractModule(parent),
      d(std::make_unique<LogbookPluginPrivate>())
{
    LogbookSettings moduleSettings;
}

LogbookPlugin::~LogbookPlugin() = default;

QString LogbookPlugin::getModuleName() const noexcept
{
    return QCoreApplication::translate("LogbookPlugin", "Logbook");
}

QWidget *LogbookPlugin::getWidget() const noexcept
{
    return d->logbookWidget.get();
}

// PROTECTED

ModuleBaseSettings &LogbookPlugin::getPluginSettings() const noexcept
{
    return d->moduleSettings;
}

// PROTECTED SLOTS

void LogbookPlugin::onRecordingStopped() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    const std::size_t sequenceNumber = flight.count();
    if (sequenceNumber > 1) {
        // Sequence starts at 1
        d->aircraftService->store(flight.getId(), sequenceNumber, flight[sequenceNumber - 1]);
    } else {
        AbstractModule::onRecordingStopped();
    }
}
