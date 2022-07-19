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
#include <vector>
#include <iterator>
#include <cstdint>
#include <cinttypes>

#include <QDateTime>
#include <QString>
#ifdef DEBUG
#include <QDebug>
#endif

#include "FlightCondition.h"
#include "Aircraft.h"
#include "FlightPlan.h"
#include "Waypoint.h"
#include "FlightSummary.h"
#include "Flight.h"

class FlightPrivate
{
public:

    FlightPrivate() noexcept
        : creationTime(QDateTime::currentDateTime()),
          userAircraftIndex(Flight::InvalidAircraftIndex)
    {
        clear(true);
    }

    std::int64_t id;
    QDateTime creationTime;
    QString title;
    QString description;
    FlightCondition flightCondition;
    std::vector<std::unique_ptr<Aircraft>> aircraft;
    int userAircraftIndex;

    inline void clear(bool withOneAircraft) noexcept {
        id = Flight::InvalidId;
        title.clear();
        description.clear();
        flightCondition.clear();
        if (aircraft.size() > 0) {
            const int aircraftCount = withOneAircraft ? 1 : 0;
            aircraft.resize(aircraftCount);
            userAircraftIndex = withOneAircraft ? 0 : Flight::InvalidAircraftIndex;
        }
        // A flight always has at least one aircraft; unless
        // it is newly allocated (the aircraft is only added in the constructor body)
        // or cleared just before loading a flight
        if (aircraft.size() > 0) {
            aircraft.at(0)->clear();
        }
    }
};

// PUBLIC

Flight::Flight(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<FlightPrivate>())
{
    // A flight always has at least one (user) aircraft
    addUserAircraft();
#ifdef DEBUG
    qDebug() << "Flight::Flight: CREATED, ID:" << d->id;
#endif
}

Flight::~Flight() noexcept
{
#ifdef DEBUG
    qDebug() << "Flight::~Flight: DELETED, ID:" << d->id;
#endif
}

void Flight::setId(std::int64_t id) noexcept
{
    d->id = id;
}

std::int64_t Flight::getId() const noexcept
{
    return d->id;
}

const QDateTime &Flight::getCreationTime() const noexcept
{
    return d->creationTime;
}

void Flight::setCreationTime(const QDateTime &creationTime) noexcept
{
    d->creationTime = creationTime;
}

const QString &Flight::getTitle() const noexcept
{
    return d->title;
}

void Flight::setTitle(const QString &title) noexcept
{
    if (d->title != title) {
        d->title = title;
        emit descriptionOrTitleChanged();
    }
}

const QString &Flight::getDescription() const noexcept
{
    return d->description;
}

void Flight::setDescription(const QString &description) noexcept
{
    if (d->description != description) {
        d->description = description;
        emit descriptionOrTitleChanged();
    }
}

void Flight::setAircraft(std::vector<std::unique_ptr<Aircraft>> aircraft) noexcept
{
    d->aircraft = std::move(aircraft);
    for (auto &aircraft : d->aircraft) {
        emit aircraftAdded(*aircraft.get());
        connectWithAircraftSignals(*aircraft.get());
    }
}

Aircraft &Flight::addUserAircraft() noexcept
{
    std::unique_ptr<Aircraft> aircraft = std::make_unique<Aircraft>();
    connectWithAircraftSignals(*aircraft.get());

    d->aircraft.push_back(std::move(aircraft));
    switchUserAircraftIndex(d->aircraft.size() - 1);
    emit aircraftAdded(*d->aircraft.back().get());
    return *d->aircraft.back().get();
}

Aircraft &Flight::getUserAircraft() const noexcept
{
    return *d->aircraft.at(d->userAircraftIndex);
}

int Flight::getUserAircraftIndex() const noexcept
{
    return d->userAircraftIndex;
}

void Flight::setUserAircraftIndex(int index) noexcept
{
    if (d->userAircraftIndex != index) {
        d->userAircraftIndex = index;
        emit userAircraftChanged(index, SkySearch::InvalidIndex);
    }
}

void Flight::switchUserAircraftIndex(int index) noexcept
{
    if (d->userAircraftIndex != index) {
        const int previousUserAircraftIndex = d->userAircraftIndex;
        d->userAircraftIndex = index;
        emit userAircraftChanged(index, previousUserAircraftIndex);
    }
}

std::int64_t Flight::deleteAircraftByIndex(int index) noexcept
{
    std::int64_t aircraftId {Aircraft::InvalidId};
    // A flight has at least one aircraft
    if (d->aircraft.size() > 1) {
        aircraftId = d->aircraft.at(index)->getId();
        if (index < d->userAircraftIndex) {
            // An aircraft with a lower index or the user aircraft index itself
            // is to be removed -> re-assign the user aircraft index accordingly
            // (but don't notify anyone about it just yet)
            reassignUserAircraftIndex(std::max(d->userAircraftIndex - 1, 0));
        } else if (index == d->userAircraftIndex) {
            // The actual user aircraft is to be removed
            setUserAircraftIndex(std::max(d->userAircraftIndex - 1, 0));
        }
        d->aircraft.erase(d->aircraft.begin() + index);
        emit aircraftRemoved(aircraftId);
    }
    return aircraftId;
}

std::size_t Flight::count() const noexcept
{
    return d->aircraft.size();
}

const FlightCondition &Flight::getFlightCondition() const noexcept
{
    return d->flightCondition;
}

void Flight::setFlightCondition(FlightCondition flightCondition) noexcept
{
    d->flightCondition = flightCondition;
    emit flightConditionChanged();
}

FlightSummary Flight::getFlightSummary() const noexcept
{
    const Aircraft &aircraft = getUserAircraft();
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();

    FlightSummary summary;
    summary.flightId = d->id;
    summary.creationDate = d->creationTime;
    summary.aircraftType = aircraftInfo.aircraftType.type;
    summary.aircraftCount = count();
    summary.startSimulationLocalTime = d->flightCondition.startLocalTime;
    summary.startSimulationZuluTime = d->flightCondition.startZuluTime;
    summary.endSimulationLocalTime = d->flightCondition.endLocalTime;
    summary.endSimulationZuluTime = d->flightCondition.endZuluTime;

    const FlightPlan &flightPlan = aircraft.getFlightPlan();
    if (flightPlan.count() > 0) {
        summary.startLocation = flightPlan[0].identifier;
    }
    if (flightPlan.count() > 1) {
        summary.endLocation = flightPlan[flightPlan.count() - 1].identifier;
    }
    summary.title = d->title;

    return summary;
}

std::int64_t Flight::getTotalDurationMSec(bool ofUserAircraft) const noexcept
{
    std::int64_t totalDuractionMSec = 0;
    if (ofUserAircraft) {
        totalDuractionMSec = getUserAircraft().getDurationMSec();
    } else {
        for (const auto &aircraft : d->aircraft) {
            totalDuractionMSec = std::max(aircraft->getDurationMSec(), totalDuractionMSec);
        }
    }
    return totalDuractionMSec;
}

QDateTime Flight::getAircraftCreationTime(const Aircraft &aircraft) const noexcept
{
    return d->creationTime.addMSecs(-aircraft.getTimeOffset());
}

QDateTime Flight::getAircraftStartLocalTime(const Aircraft &aircraft) const noexcept
{
    return d->flightCondition.startLocalTime.addMSecs(-aircraft.getTimeOffset());
}

QDateTime Flight::getAircraftStartZuluTime(const Aircraft &aircraft) const noexcept
{
    return d->flightCondition.startZuluTime.addMSecs(-aircraft.getTimeOffset());
}

void Flight::clear(bool withOneAircraft) noexcept
{
    d->creationTime = QDateTime::currentDateTime();
    d->clear(withOneAircraft);
    if (withOneAircraft) {
        // Only emit the signals if the flight has at least one aircraft
        // (but e.g. not shortly before loading a new flight from the logbook)
        emit cleared();
        emit descriptionOrTitleChanged();
    }
}

Flight::Iterator Flight::begin() noexcept
{
    return d->aircraft.begin();
}

Flight::Iterator Flight::end() noexcept
{
    return d->aircraft.end();
}

const Flight::Iterator Flight::begin() const noexcept
{
    return d->aircraft.begin();
}

const Flight::Iterator Flight::end() const noexcept
{
    return d->aircraft.end();
}

// OPERATORS

Aircraft &Flight::operator[](std::size_t index) noexcept
{
    return *d->aircraft[index];
}

const Aircraft &Flight::operator[](std::size_t index) const noexcept
{
    return *d->aircraft[index];
}

// PRIVATE

inline void Flight::connectWithAircraftSignals(Aircraft &aircraft)
{
    connect(&aircraft, &Aircraft::infoChanged,
            this, &Flight::aircraftInfoChanged);
    connect(&aircraft, &Aircraft::tailNumberChanged,
            this, &Flight::tailNumberChanged);
    connect(&aircraft, &Aircraft::timeOffsetChanged,
            this, &Flight::timeOffsetChanged);
}

void Flight::reassignUserAircraftIndex(int index) noexcept
{
    d->userAircraftIndex = index;
}
