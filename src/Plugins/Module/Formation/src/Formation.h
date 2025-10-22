/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#include <utility>

#include <Model/InitialPosition.h>
#include <Model/PositionData.h>
#include <Model/AttitudeData.h>

namespace Formation
{
    /*!
     * The initial horizontal distance of the formation aircraft, relative to the current user aircraft.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct HorizontalDistance: std::uint8_t {
        First = 0,
        VeryClose = First,
        Close,
        Nearby,
        Far,
        VeryFar,
        Last = VeryFar
    };

    /*!
     * The initial vertical distance of the formation aircraft, relative to the current user aircraft.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct VerticalDistance: std::uint8_t {
        First = 0,
        Below = First,
        JustBelow,
        Level,
        JustAbove,
        Above,
        Last = Above
    };

    /*!
     * The initial bearing of the formation aircraft, relative to the user aircraft.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct Bearing: std::uint8_t {
        First = 0,
        North = First,
        NorthNorthEast,
        NorthEast,
        EastNorthEast,
        East,
        EastSouthEast,
        SouthEast,
        SouthSouthEast,
        South,
        SouthSouthWest,
        SouthWest,
        WestSouthWest,
        West,
        WestNorthWest,
        NorthWest,
        NorthNorthWest,
        Last = NorthNorthWest
    };

    InitialPosition calculateInitialRelativePositionToUserAircraft(HorizontalDistance horizontalDistance, VerticalDistance verticalDistance, Bearing bearing, std::int64_t timestamp) noexcept;
    std::pair<PositionData, AttitudeData> calculateRelativePositionToUserAircraft(HorizontalDistance horizontalDistance, VerticalDistance verticalDistance, Bearing bearing, std::int64_t timestamp) noexcept;
};

#endif // FORMATION_H
