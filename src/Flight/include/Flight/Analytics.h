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
#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <memory>
#include <utility>
#include <cstdint>

#include <QtGlobal>

#include "FlightLib.h"

class Aircraft;
struct PositionData;

struct AnalyticsPrivate;

/*!
 * Provides basic flight path & events analytics.
 */
class FLIGHT_API Analytics
{
public:
    Analytics(const Aircraft &aircraft);
    Analytics(const Analytics &rhs) = delete;
    Analytics(Analytics &&rhs) noexcept;
    Analytics &operator=(const Analytics &rhs) = delete;
    Analytics &operator=(Analytics &&rhs) noexcept;
    ~Analytics();

    /*!
     * Returns the heading of the aircraft when its movement first exceeds
     * a certain distance threshold (when the aircraft starts moving while typically
     * still on the ground). The movement is purely calculated based on the distance
     * between the positions.
     *
     * \return the heading of the first aircraft movement
     */
    std::pair<std::int64_t, double> firstMovementHeading() const noexcept;

    PositionData closestPosition(double latitude, double longitude) const noexcept;

private:
    std::unique_ptr<AnalyticsPrivate> d;
};

#endif // ANALYTICS_H
