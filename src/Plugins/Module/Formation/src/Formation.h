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
#ifndef FORMATION_H
#define FORMATION_H

#include <cstdint>

#include <Model/InitialPosition.h>
#include <Model/PositionData.h>

namespace Formation
{
    /*!
     * The initial horizontal distance of the formation aircraft, relative to the current user aircraft.
     *
     * Implementation note: those values act as actual IDs that get persisted in the database.
     */
    enum struct HorizontalDistance {
        VeryClose = 0,
        Close,
        Nearby,
        Far,
        VeryFar
    };

    /*!
     * The initial vertical distance of the formation aircraft, relative to the current user aircraft.
     *
     * Implementation note: those values act as actual IDs that get persisted in the database.
     */
    enum struct VerticalDistance {
        Below = 0,
        JustBelow = 1,
        Level = 2,
        JustAbove = 3,
        Above = 4
    };

    /*!
     * The initial bearing of the formation aircraft, relative to the user aircraft.
     *
     * Implementation note: those values act as actual IDs that get persisted in the database.
     */
    enum struct Bearing {
        North = 0,
        NorthNorthEast = 1,
        NorthEast = 2,
        EastNorthEast = 3,
        East = 4,
        EastSouthEast = 5,
        SouthEast = 6,
        SouthSouthEast = 7,
        South = 8,
        SouthSouthWest = 9,
        SouthWest = 10,
        WestSouthWest = 11,
        West = 12,
        WestNorthWest = 13,
        NorthWest = 14,
        NorthNorthWest = 15
    };

    InitialPosition calculateInitialRelativePositionToUserAircraft(HorizontalDistance horizontalDistance, VerticalDistance verticalDistance, Bearing relativePosition, std::int64_t timestamp) noexcept;
    PositionData calculateRelativePositionToUserAircraft(HorizontalDistance horizontalDistance, VerticalDistance verticalDistance, Bearing relativePosition, std::int64_t timestamp) noexcept;
};

#endif // FORMATION_H
