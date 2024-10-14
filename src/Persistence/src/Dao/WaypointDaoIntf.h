/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef WAYPOINTDAOINTF_H
#define WAYPOINTDAOINTF_H

#include <cstdint>

class FlightPlan;
class Waypoint;

class WaypointDaoIntf
{
public:
    WaypointDaoIntf() = default;
    WaypointDaoIntf(const WaypointDaoIntf &rhs) = delete;
    WaypointDaoIntf(WaypointDaoIntf &&rhs) = default;
    WaypointDaoIntf &operator=(const WaypointDaoIntf &rhs) = delete;
    WaypointDaoIntf &operator=(WaypointDaoIntf &&rhs) = default;
    virtual ~WaypointDaoIntf() = default;

    /*!
     * Persists the \p flightPlan.
     *
     * \param aircraftId
     *        the aircraft the \p flightPlan belongs to
     * \param flightPlan
     *        the FlightPlan data to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool add(std::int64_t aircraftId, const FlightPlan &flightPlan) const noexcept = 0;
    virtual bool getByAircraftId(std::int64_t aircraftId, FlightPlan &flightPlan) const noexcept = 0;
    virtual bool deleteByFlightId(std::int64_t flightId) const noexcept = 0;
    virtual bool deleteByAircraftId(std::int64_t aircraftId) const noexcept = 0;
};

#endif // WAYPOINTDAOINTF_H
