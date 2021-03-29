/**
 * Sky Dolly - The black sheep for your flight recordings
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

#include "FlightCondition.h"
#include "Aircraft.h"
#include "Scenario.h"

class ScenarioPrivate
{
public:

    ScenarioPrivate() noexcept
    {}

    ~ScenarioPrivate() noexcept
    {}

    FlightCondition flightCondition;
    std::vector<std::unique_ptr<Aircraft>> aircrafts;

};

// PUBLIC

Scenario::Scenario(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<ScenarioPrivate>())
{
    // The scenario may support several aircrafts, but for now there will be always
    // exactly one user aircraft
    std::unique_ptr<Aircraft> userAircraft = std::make_unique<Aircraft>();
    d->aircrafts.push_back(std::move(userAircraft));

    frenchConnection();
}

Scenario::~Scenario() noexcept
{
}

const Aircraft &Scenario::getUserAircraftConst() const noexcept
{
    return *(*d->aircrafts.cbegin());
}

Aircraft &Scenario::getUserAircraft() const noexcept
{
    return *(*d->aircrafts.cbegin());
}

void Scenario::setFlightCondition(FlightCondition flightCondition) noexcept
{
    d->flightCondition = flightCondition;
    emit flightConditionChanged();
}

const FlightCondition &Scenario::getFlightConditionConst() const noexcept
{
    return d->flightCondition;
}

FlightCondition &Scenario::getFlightCondition() const noexcept
{
    return d->flightCondition;
}

qint64 Scenario::getTotalDuration() const noexcept
{
    // For now the total duration is the duration of the
    // (one and only) user aircraft
    return d->aircrafts.at(0)->getDuration();
}

// PRIVATE

void Scenario::frenchConnection() noexcept
{
    Aircraft &userAircraft = getUserAircraft();
    connect(&userAircraft, &Aircraft::infoChanged,
            this, &Scenario::aircraftInfoChanged);
    connect(&userAircraft, &Aircraft::dataChanged,
            this, &Scenario::aircraftDataChanged);
}
