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

#include <QObject>
#include <QVector>

#include "Waypoint.h"
#include "FlightPlan.h"

class FlightPlanPrivate
{
public:
    FlightPlanPrivate() noexcept
    {}

    QVector<Waypoint> waypoints;
};

// PUBLIC

FlightPlan::FlightPlan(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<FlightPlanPrivate>())
{}

FlightPlan::~FlightPlan() noexcept
{}

void FlightPlan::add(const Waypoint &waypoint) noexcept
{
    d->waypoints.append(waypoint);
    emit waypointAdded(waypoint);
}

void FlightPlan::update(int index, const Waypoint &waypoint) noexcept
{
    Waypoint currentWaypoint = d->waypoints.at(index);
    bool changed = false;
    if (index >= 0 && index < d->waypoints.count()) {
        if (currentWaypoint.timestamp != waypoint.timestamp) {
            currentWaypoint.timestamp = waypoint.timestamp;
            changed = true;
        }
        if (currentWaypoint.latitude != waypoint.latitude) {
            currentWaypoint.latitude = waypoint.latitude;
            changed = true;
        }
        if (currentWaypoint.longitude != waypoint.longitude) {
            currentWaypoint.longitude = waypoint.longitude;
            changed = true;
        }
        if (currentWaypoint.altitude != waypoint.altitude) {
            currentWaypoint.altitude = waypoint.altitude;
            changed = true;
        }
        if (currentWaypoint.localTime != waypoint.localTime) {
            currentWaypoint.localTime = waypoint.localTime;
            changed = true;
        }
        if (currentWaypoint.zuluTime != waypoint.zuluTime) {
            currentWaypoint.zuluTime = waypoint.zuluTime;
            changed = true;
        }
    }
    if (changed) {
        d->waypoints[index] = currentWaypoint;
        emit waypointUpdated(index, currentWaypoint);
    }
}

const QVector<Waypoint> &FlightPlan::getAllConst() const noexcept
{
    return d->waypoints;
}

void FlightPlan::clear() noexcept
{
    d->waypoints.clear();
    emit waypointsCleared();
}
