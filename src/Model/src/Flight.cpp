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
#include <vector>
#include <iterator>
#include <cstdint>
#include <cinttypes>

#include <QDateTime>
#include <QString>

#include "FlightCondition.h"
#include "Aircraft.h"
#include "Flight.h"

namespace
{
    constexpr int InvalidAircraftIndex = -1;
}

class FlightPrivate
{
public:

    FlightPrivate() noexcept
        : creationTime(QDateTime::currentDateTime()),
          userAircraftIndex(InvalidAircraftIndex)
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
            userAircraftIndex = withOneAircraft ? 0 : InvalidAircraftIndex;
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
    qDebug("Flight::Flight:  CREATED, ID: %" PRId64, d->id);
#endif
}

Flight::~Flight() noexcept
{
#ifdef DEBUG
    qDebug("Flight::~Flight:  DELETED, ID: %" PRId64, d->id);
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
    setUserAircraftIndex(d->aircraft.size() - 1);
    emit aircraftAdded(*d->aircraft.back().get());
    return *d->aircraft.back().get();
}

const Aircraft &Flight::getUserAircraftConst() const noexcept
{
    return *d->aircraft.at(d->userAircraftIndex);
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
        emit userAircraftChanged();
    }
}

std::int64_t Flight::deleteAircraftByIndex(int index) noexcept
{
    std::int64_t aircraftId;
    // A flight has at least one aircraft
    if (d->aircraft.size() > 1) {
        setUserAircraftIndex(qMax(d->userAircraftIndex - 1, 0));
        aircraftId  = d->aircraft.at(index)->getId();
        std::int64_t simulationObjectId = d->aircraft.at(index)->getSimulationObjectId();
        d->aircraft.erase(d->aircraft.begin() + index);
        emit aircraftDeleted(simulationObjectId);
    } else {
        aircraftId = Aircraft::InvalidId;
    }
    return aircraftId;
}

std::size_t Flight::count() const noexcept
{
    return d->aircraft.size();
}

const FlightCondition &Flight::getFlightConditionConst() const noexcept
{
    return d->flightCondition;
}

void Flight::setFlightCondition(FlightCondition flightCondition) noexcept
{
    d->flightCondition = flightCondition;
    emit flightConditionChanged();
}

std::int64_t Flight::getTotalDurationMSec(bool ofUserAircraft) const noexcept
{
    std::int64_t totalDuractionMSec = 0;
    if (ofUserAircraft) {
        totalDuractionMSec = getUserAircraftConst().getDurationMSec();
    } else {
        for (const auto &aircraft : d->aircraft) {
            totalDuractionMSec = qMax(aircraft->getDurationMSec(), totalDuractionMSec);
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
        emit flightCleared();
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

Aircraft& Flight::operator[](std::size_t index) noexcept
{
    return *d->aircraft[index];
}

const Aircraft& Flight::operator[](std::size_t index) const noexcept
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
