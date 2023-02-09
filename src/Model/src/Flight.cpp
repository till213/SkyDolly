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
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include <cstdint>
#include <cinttypes>

#include <QDateTime>
#include <QString>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Const.h>
#include "FlightCondition.h"
#include "Aircraft.h"
#include "FlightPlan.h"
#include "Waypoint.h"
#include "FlightSummary.h"
#include "FlightData.h"
#include "Flight.h"

struct FlightPrivate
{
    FlightPrivate() noexcept
    {
        flightData.clear(true);
    }

    FlightData flightData;
};

// PUBLIC

Flight::Flight(FlightData flightData, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<FlightPrivate>())
{
    d->flightData = std::move(flightData);
}

Flight::Flight(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<FlightPrivate>())
{
    // A flight always has at least one (user) aircraft
    addUserAircraft();
    frenchConnection();
}

Flight::~Flight() = default;

void Flight::fromFlightData(FlightData flightData) noexcept
{
    d->flightData = std::move(flightData);
}

void Flight::fromFlightData(FlightData &&flightData) noexcept
{
    d->flightData = std::move(flightData);
}

FlightData &Flight::getFlightData() const noexcept
{
    return d->flightData;
}

void Flight::setId(std::int64_t id) noexcept
{
    d->flightData.id = id;
}

std::int64_t Flight::getId() const noexcept
{
    return d->flightData.id;
}

const QDateTime &Flight::getCreationTime() const noexcept
{
    return d->flightData.creationTime;
}

void Flight::setCreationTime(const QDateTime &creationTime) noexcept
{
    d->flightData.creationTime = creationTime;
}

const QString &Flight::getTitle() const noexcept
{
    return d->flightData.title;
}

void Flight::setTitle(const QString &title) noexcept
{
    if (d->flightData.title != title) {
        d->flightData.title = title;
        emit descriptionOrTitleChanged();
    }
}

const QString &Flight::getDescription() const noexcept
{
    return d->flightData.description;
}

void Flight::setDescription(const QString &description) noexcept
{
    if (d->flightData.description != description) {
        d->flightData.description = description;
        emit descriptionOrTitleChanged();
    }
}

void Flight::setAircraft(std::vector<Aircraft> &&aircraft) noexcept
{
    d->flightData.aircraft = std::move(aircraft);
    for (auto &aircraft : d->flightData.aircraft) {
        emit aircraftAdded(aircraft);
    }
}

Aircraft &Flight::addUserAircraft() noexcept
{
    d->flightData.aircraft.emplace_back();
    switchUserAircraftIndex(static_cast<int>(d->flightData.aircraft.size()) - 1);
    emit aircraftAdded(d->flightData.aircraft.back());
    return d->flightData.aircraft.back();
}

Aircraft &Flight::getUserAircraft() const noexcept
{
    return d->flightData.getUserAircraft();
}

int Flight::getAircraftIndex(const Aircraft &aircraft) const noexcept
{
    int index {Const::InvalidIndex};
    const auto it = std::find_if(d->flightData.aircraft.cbegin(), d->flightData.aircraft.cend(),
                                 [&aircraft](const Aircraft &a) { return a.getId() == aircraft.getId(); });
    if (it != d->flightData.aircraft.cend()) {
        index = static_cast<int>(std::distance(d->flightData.aircraft.cbegin(), it));
    }
    return index;
}

int Flight::getUserAircraftIndex() const noexcept
{
    return d->flightData.userAircraftIndex;
}

void Flight::setUserAircraftIndex(int index) noexcept
{
    if (d->flightData.userAircraftIndex != index) {
        d->flightData.userAircraftIndex = index;
        emit userAircraftChanged(index, Const::InvalidIndex);
    }
}

void Flight::switchUserAircraftIndex(int index) noexcept
{
    if (d->flightData.userAircraftIndex != index) {
        const int previousUserAircraftIndex = d->flightData.userAircraftIndex;
        d->flightData.userAircraftIndex = index;
        emit userAircraftChanged(index, previousUserAircraftIndex);
    }
}

std::int64_t Flight::removeAircraftByIndex(int index) noexcept
{
    std::int64_t aircraftId {Const::InvalidId};
    // A flight has at least one aircraft
    if (d->flightData.aircraft.size() > 1) {
        aircraftId = d->flightData.aircraft.at(index).getId();
        if (index < d->flightData.userAircraftIndex) {
            // An aircraft with a lower index or the user aircraft index itself
            // is to be removed -> re-assign the user aircraft index accordingly
            // (but don't notify anyone about it just yet)
            reassignUserAircraftIndex(std::max(d->flightData.userAircraftIndex - 1, 0));
        } else if (index == d->flightData.userAircraftIndex) {
            // The actual user aircraft is to be removed
            setUserAircraftIndex(std::max(d->flightData.userAircraftIndex - 1, 0));
        }
        d->flightData.aircraft.erase(d->flightData.aircraft.begin() + index);
        emit aircraftRemoved(aircraftId);
    }
    return aircraftId;
}

std::int64_t Flight::removeLastAircraft() noexcept
{
    std::int64_t aircraftId {Const::InvalidId};
    const std::size_t size = d->flightData.aircraft.size();
    if (size > 0) {
        aircraftId = removeAircraftByIndex(static_cast<int>(d->flightData.aircraft.size()) - 1);
    }
    return aircraftId;
}

std::size_t Flight::count() const noexcept
{
    return d->flightData.aircraft.size();
}

void Flight::addWaypoint(const Waypoint &waypoint) noexcept
{
    getUserAircraft().getFlightPlan().add(waypoint);
    emit waypointAdded(waypoint);
}

void Flight::updateWaypoint(int index, const Waypoint &waypoint) noexcept
{
    getUserAircraft().getFlightPlan().update(index, waypoint);
    emit waypointUpdated(index, waypoint);
}

void Flight::clearWaypoints() noexcept
{
    getUserAircraft().getFlightPlan().clear();
    emit waypointsCleared();
}

const FlightCondition &Flight::getFlightCondition() const noexcept
{
    return d->flightData.flightCondition;
}

void Flight::setFlightCondition(FlightCondition flightCondition) noexcept
{
    d->flightData.flightCondition = std::move(flightCondition);
    emit flightConditionChanged();
}

FlightSummary Flight::getFlightSummary() const noexcept
{
    const Aircraft &aircraft = getUserAircraft();
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();

    FlightSummary summary;
    summary.flightId = d->flightData.id;
    summary.creationDate = d->flightData.creationTime;
    summary.aircraftType = aircraftInfo.aircraftType.type;
    summary.aircraftCount = count();
    summary.startSimulationLocalTime = d->flightData.flightCondition.startLocalTime;
    summary.startSimulationZuluTime = d->flightData.flightCondition.startZuluTime;
    summary.endSimulationLocalTime = d->flightData.flightCondition.endLocalTime;
    summary.endSimulationZuluTime = d->flightData.flightCondition.endZuluTime;

    const FlightPlan &flightPlan = aircraft.getFlightPlan();
    if (flightPlan.count() > 0) {
        summary.startLocation = flightPlan[0].identifier;
    }
    if (flightPlan.count() > 1) {
        summary.endLocation = flightPlan[flightPlan.count() - 1].identifier;
    }
    summary.title = d->flightData.title;

    return summary;
}

std::int64_t Flight::getTotalDurationMSec(bool ofUserAircraft) const noexcept
{
    return d->flightData.getTotalDurationMSec(ofUserAircraft);
}

QDateTime Flight::getAircraftCreationTime(const Aircraft &aircraft) const noexcept
{
    return d->flightData.getAircraftCreationTime(aircraft);
}

QDateTime Flight::getAircraftStartLocalTime(const Aircraft &aircraft) const noexcept
{
    return d->flightData.getAircraftStartLocalTime(aircraft);
}

QDateTime Flight::getAircraftStartZuluTime(const Aircraft &aircraft) const noexcept
{
    return d->flightData.getAircraftStartZuluTime(aircraft);
}

void Flight::clear(bool withOneAircraft) noexcept
{
    d->flightData.creationTime = QDateTime::currentDateTime();
    d->flightData.clear(withOneAircraft);
    if (withOneAircraft) {
        // Only emit the signals if the flight has at least one aircraft
        // (but e.g. not shortly before loading a new flight from the logbook)
        emit cleared();
        emit waypointsCleared();
        emit descriptionOrTitleChanged();
    }
}

Flight::Iterator Flight::begin() noexcept
{
    return d->flightData.begin();
}

Flight::Iterator Flight::end() noexcept
{
    return d->flightData.end();
}

Flight::ConstIterator Flight::begin() const noexcept
{
    return d->flightData.begin();
}

Flight::ConstIterator Flight::end() const noexcept
{
    return d->flightData.end();
}

// OPERATORS

Aircraft &Flight::operator[](std::size_t index) noexcept
{
    return d->flightData[index];
}

const Aircraft &Flight::operator[](std::size_t index) const noexcept
{
    return d->flightData[index];
}

// PRIVATE

 void Flight::frenchConnection()
 {
     connect(this, &Flight::timeOffsetChanged,
             this, &Flight::aircraftInfoChanged);
     connect(this, &Flight::tailNumberChanged,
             this, &Flight::aircraftInfoChanged);
 }

void Flight::reassignUserAircraftIndex(int index) noexcept
{
    d->flightData.userAircraftIndex = index;
}
