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

#include <QObject>
#include <QCoreApplication>
#include <QDateTime>
#include <QDate>
#include <QTime>

#include <Kernel/Unit.h>
#include <Model/Location.h>
#include <Model/TimeZoneInfo.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/Service/EnumerationService.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "LocationWidget.h"
#include "LocationSettings.h"
#include "LocationPlugin.h"

namespace
{
    constexpr int MorningMSecsSinceMidnight = 8 * Unit::MillisecondsPerMinute * Unit::MinutesPerHour;
    constexpr int NoonMSecsSinceMidnight = 12 * Unit::MillisecondsPerMinute * Unit::MinutesPerHour;
    constexpr int AfternoonMSecsSinceMidnight = 16 * Unit::MillisecondsPerMinute * Unit::MinutesPerHour;
    constexpr int EveningMSecsSinceMidnight = 18 * Unit::MillisecondsPerMinute * Unit::MinutesPerHour;
    constexpr int NightMSecsSinceMidnight = 21 * Unit::MillisecondsPerMinute * Unit::MinutesPerHour;
    constexpr int MidnightMSecsSinceMidnight = 0 * Unit::MillisecondsPerMinute * Unit::MinutesPerHour;
}

struct LocationPluginPrivate
{
    enum struct Mode: std::uint8_t
    {
        Add,
        Update
    };

    LocationSettings moduleSettings;
    std::unique_ptr<LocationWidget> locationWidget {std::make_unique<LocationWidget>(moduleSettings)};
    const std::int64_t EngineEventStartId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventStartSymId).id()};
    const std::int64_t EngineEventStopId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventStopSymId).id()};
    Mode mode {Mode::Add};
    QDateTime selectedDateTime;
};

// PUBLIC

LocationPlugin::LocationPlugin(QObject *parent) noexcept
    : AbstractModule {parent},
      d {std::make_unique<LocationPluginPrivate>()}
{
    restoreSettings(QUuid(Const::LocationModuleUuid));
    frenchConnection();
}

LocationPlugin::~LocationPlugin()
{
    storeSettings(QUuid(Const::LocationModuleUuid));
};

QUuid LocationPlugin::getUuid() const noexcept
{
    static const QUuid uuid {Const::LocationModuleUuid};
    return uuid;
}

QString LocationPlugin::getModuleName() const noexcept
{
    return QCoreApplication::translate("LocationPlugin", "Location");
}

QWidget *LocationPlugin::getWidget() const noexcept
{
    return d->locationWidget.get();
}

// PROTECTED

ModuleBaseSettings &LocationPlugin::getModuleSettings() const noexcept
{
    return d->moduleSettings;
}

// PRIVATE

void LocationPlugin::frenchConnection() noexcept
{
    // Connection
    auto &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::locationReceived,
            this, &LocationPlugin::onLocationReceived);
    connect(&skyConnectManager, &SkyConnectManager::timeZoneInfoReceived,
            this, &LocationPlugin::onTimeZoneInfoReceived);

    // Location widget
    connect(d->locationWidget.get(), &LocationWidget::doCaptureLocation,
            this, &LocationPlugin::captureLocation);
    connect(d->locationWidget.get(), &LocationWidget::doUpdateLocation,
            this, &LocationPlugin::updateLocation);
    connect(d->locationWidget.get(), &LocationWidget::teleportTo,
            this, &LocationPlugin::teleportTo);
}

QDateTime LocationPlugin::getSelectedDateTime(const QDateTime &dateTime) const noexcept
{
    QDate date;
    QTime time;
    switch (d->moduleSettings.getDateSelection())
    {
    case LocationSettings::DateSelection::Today:
        date = QDate::currentDate();
        break;
    case LocationSettings::DateSelection::Date:
        date = dateTime.date();
        break;
    case LocationSettings::DateSelection::DateTime:
        date = dateTime.date();
        time = dateTime.time();
        break;
    }

    if (d->moduleSettings.getDateSelection() != LocationSettings::DateSelection::DateTime) {
        if (d->moduleSettings.getTimeSelection() == LocationSettings::TimeSelection::Now) {
            time = QTime::currentTime();
        }
    }
    return {date, time};
}

QDateTime LocationPlugin::getSelectedDateTime(const TimeZoneInfo &timeZoneInfo) const noexcept
{
    QDateTime dateTime;
    QTime time;
    if (d->moduleSettings.getDateSelection() != LocationSettings::DateSelection::DateTime) {
        switch (d->moduleSettings.getTimeSelection()) {
        case LocationSettings::TimeSelection::Now:
            dateTime = d->selectedDateTime.addSecs(timeZoneInfo.timeZoneOffsetSeconds);
            break;
        case LocationSettings::TimeSelection::Morning:
            dateTime = QDateTime(d->selectedDateTime.date(), QTime::fromMSecsSinceStartOfDay(::MorningMSecsSinceMidnight).addMSecs(static_cast<int>(timeZoneInfo.timeZoneOffsetSeconds * Unit::MillisecondsPerSecond)));
            break;
        case LocationSettings::TimeSelection::Noon:
            dateTime = QDateTime(d->selectedDateTime.date(), QTime::fromMSecsSinceStartOfDay(::NoonMSecsSinceMidnight).addMSecs(static_cast<int>(timeZoneInfo.timeZoneOffsetSeconds * Unit::MillisecondsPerSecond)));
            break;
        case LocationSettings::TimeSelection::Afternoon:
            dateTime = QDateTime(d->selectedDateTime.date(), QTime::fromMSecsSinceStartOfDay(::AfternoonMSecsSinceMidnight).addMSecs(static_cast<int>(timeZoneInfo.timeZoneOffsetSeconds * Unit::MillisecondsPerSecond)));
            break;
        case LocationSettings::TimeSelection::Evening:
            dateTime = QDateTime(d->selectedDateTime.date(), QTime::fromMSecsSinceStartOfDay(::EveningMSecsSinceMidnight).addMSecs(static_cast<int>(timeZoneInfo.timeZoneOffsetSeconds * Unit::MillisecondsPerSecond)));
            break;
        case LocationSettings::TimeSelection::Night:
            dateTime = QDateTime(d->selectedDateTime.date(), QTime::fromMSecsSinceStartOfDay(::NightMSecsSinceMidnight).addMSecs(static_cast<int>(timeZoneInfo.timeZoneOffsetSeconds * Unit::MillisecondsPerSecond)));
            break;
        case LocationSettings::TimeSelection::Midnight:
            dateTime = QDateTime(d->selectedDateTime.date(), QTime::fromMSecsSinceStartOfDay(::MidnightMSecsSinceMidnight).addMSecs(static_cast<int>(timeZoneInfo.timeZoneOffsetSeconds * Unit::MillisecondsPerSecond)));
            break;
        case LocationSettings::TimeSelection::Sunrise:
            dateTime = QDateTime(d->selectedDateTime.date(), QTime::fromMSecsSinceStartOfDay(static_cast<int>(timeZoneInfo.zuluSunriseTimeSeconds * Unit::MillisecondsPerSecond)));
            break;
        case LocationSettings::TimeSelection::Sunset:
            dateTime = QDateTime(d->selectedDateTime.date(), QTime::fromMSecsSinceStartOfDay(static_cast<int>(timeZoneInfo.zuluSunsetTimeSeconds * Unit::MillisecondsPerSecond)));
            break;
        }
    } else {
        dateTime = d->selectedDateTime.addSecs(timeZoneInfo.timeZoneOffsetSeconds);
    }
    return dateTime;
}

// PRIVATE SLOTS

void LocationPlugin::captureLocation() noexcept
{
    d->mode = LocationPluginPrivate::Mode::Add;
    SkyConnectManager::getInstance().requestLocation();
}

void LocationPlugin::updateLocation() noexcept
{
    d->mode = LocationPluginPrivate::Mode::Update;
    SkyConnectManager::getInstance().requestLocation();
}

void LocationPlugin::teleportTo(const Location &location, const QDateTime &dateTime) noexcept
{
    const InitialPosition initialPosition = location.toInitialPosition();
    auto &skyConnectManager = SkyConnectManager::getInstance();
    skyConnectManager.setUserAircraftInitialPosition(initialPosition);
    SkyConnectIntf::SimulationEvent event {SkyConnectIntf::SimulationEvent::None};

    if (location.engineEventId == d->EngineEventStartId) {
        event = SkyConnectIntf::SimulationEvent::EngineStart;
    } else if (location.engineEventId == d->EngineEventStopId) {
        event = SkyConnectIntf::SimulationEvent::EngineStop;
    }

    if (event != SkyConnectIntf::SimulationEvent::None) {
        skyConnectManager.sendSimulationEvent(event);
    }
    d->selectedDateTime = getSelectedDateTime(dateTime);
    // Set the date before requesting sunset/sunrise/time offset times
    if (d->selectedDateTime.isValid()) {
        skyConnectManager.sendZuluDateTime(d->selectedDateTime);
    }
    skyConnectManager.requestTimeZoneInfo();
}

void LocationPlugin::onLocationReceived(Location location) noexcept
{
    switch (d->mode)
    {
    case LocationPluginPrivate::Mode::Add:
        d->locationWidget->addLocation(std::move(location));
        break;
    case LocationPluginPrivate::Mode::Update:
        d->locationWidget->updateLocation(location);
        break;
    }
}

void LocationPlugin::onTimeZoneInfoReceived(TimeZoneInfo timeZoneInfo) const noexcept
{
    QDateTime dateTime = getSelectedDateTime(timeZoneInfo);
    if (dateTime.isValid()) {
        auto &skyConnectManager = SkyConnectManager::getInstance();
        skyConnectManager.sendZuluDateTime(dateTime);
    }
}
