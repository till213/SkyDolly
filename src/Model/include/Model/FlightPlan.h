/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef FLIGHTPLAN_H
#define FLIGHTPLAN_H

#include <cstddef>
#include <memory>
#include <vector>
#include <iterator>

#include "Waypoint.h"
#include "ModelLib.h"

struct Waypoint;
struct FlightPlanPrivate;

class MODEL_API FlightPlan final
{
public:
    FlightPlan() noexcept;
    FlightPlan(const FlightPlan &rhs) = delete;
    FlightPlan(FlightPlan &&rhs) noexcept;
    FlightPlan &operator=(const FlightPlan &rhs) = delete;
    FlightPlan &operator=(FlightPlan &&rhs) noexcept;
    ~FlightPlan();

    void add(Waypoint waypoint) noexcept;
    void update(int index, const Waypoint &waypoint) noexcept;
    std::size_t count() const noexcept;
    void reserve(std::size_t n);
    void clear() noexcept;

    using Iterator = std::vector<Waypoint>::iterator;

    Iterator begin() noexcept;
    Iterator end() noexcept;
    const Iterator begin() const noexcept;
    const Iterator end() const noexcept;

    Waypoint &operator[](std::size_t index) noexcept;
    const Waypoint &operator[](std::size_t index) const noexcept;

private:
    std::unique_ptr<FlightPlanPrivate> d;
};

#endif // FLIGHTPLAN_H
