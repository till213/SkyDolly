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
#include <cstdint>

#include <Kernel/SkyMath.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>

#include "Formation.h"

InitialPosition Formation::calculateInitialRelativePositionToUserAircraft(HorizontalDistance horizontalDistance, VerticalDistance verticalDistance, RelativePosition relativePosition, std::int64_t timestamp) noexcept
{
    InitialPosition initialPosition;

    const PositionData &relativePositionData = calculateRelativePositionToUserAircraft(horizontalDistance, verticalDistance, relativePosition, timestamp);
    if (!relativePositionData.isNull()) {
        initialPosition.fromPositionData(relativePositionData);
        if (timestamp == 0) {
            const Flight &flight = Logbook::getInstance().getCurrentFlight();
            const Aircraft &userAircraft = flight.getUserAircraft();
            const AircraftInfo &aircraftInfo = userAircraft.getAircraftInfo();
            initialPosition.onGround =  aircraftInfo.startOnGround;
        } else {
            initialPosition.onGround = false;
        }
    }
    return initialPosition;
}

PositionData Formation::calculateRelativePositionToUserAircraft(HorizontalDistance horizontalDistance, VerticalDistance verticalDistance, RelativePosition relativePosition, std::int64_t timestamp) noexcept
{
    PositionData initialPosition;

    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const Aircraft &userAircraft = flight.getUserAircraft();
    Position &position = userAircraft.getPosition();
    const PositionData &positionData = timestamp == 0 ? position.getFirst() : position.interpolate(timestamp, TimeVariableData::Access::Seek);
    if (!positionData.isNull()) {

        const AircraftInfo &aircraftInfo = userAircraft.getAircraftInfo();
        const AircraftType &aircraftType = aircraftInfo.aircraftType;

        // Copy pitch, bank, heading and velocity
        initialPosition = positionData;

        // Horizontal distance [feet]
        double distance {0.0};
        switch (horizontalDistance) {
        case HorizontalDistance::VeryClose:
            // Aircraft one wing apart
            distance = 1.5 * aircraftType.wingSpan;
            break;
        case HorizontalDistance::Close:
            // Aircraft one wingspan
            distance = 2.0 * aircraftType.wingSpan;
            break;
        case HorizontalDistance::Nearby:
            // Aircraft two wingspans
            distance = 3.0 * aircraftType.wingSpan;
            break;
        case HorizontalDistance::Far:
            // Aircraft three wingspans apart
            distance = 4.0 * aircraftType.wingSpan;
            break;
        case HorizontalDistance::VeryFar:
            // Aircraft four wingspans apart
            distance = 5.0 * aircraftType.wingSpan;
            break;
        }

        // Vertical distance [feet]
        const SkyMath::Coordinate sourcePosition(positionData.latitude, positionData.longitude);
        double deltaAltitude {0.0};
        switch (verticalDistance) {
        case VerticalDistance::Below:
            deltaAltitude = -distance;
            break;
        case VerticalDistance::JustBelow:
            deltaAltitude = -distance / 2.0;
            break;
        case VerticalDistance::Level:
            deltaAltitude = 0.0;
            break;
        case VerticalDistance::JustAbove:
            deltaAltitude = +distance / 2.0;
            break;
        case VerticalDistance::Above:
            deltaAltitude = +distance;
            break;
        }
        const double altitude = positionData.altitude + deltaAltitude;

        // Degrees
        double bearing {0.0};
        switch (relativePosition) {
        case RelativePosition::North:
            bearing = 0.0;
            break;
        case RelativePosition::NorthNorthEast:
            bearing = 22.5;
            break;
        case RelativePosition::NorthEast:
            bearing = 45.0;
            break;
        case RelativePosition::EastNorthEast:
            bearing = 67.5;
            break;
        case RelativePosition::East:
            bearing = 90.0;
            break;
        case RelativePosition::EastSouthEast:
            bearing = 112.5;
            break;
        case RelativePosition::SouthEast:
            bearing = 135.0;
            break;
        case RelativePosition::SouthSouthEast:
            bearing = 157.5;
            break;
        case RelativePosition::South:
            bearing = 180.0;
            break;
        case RelativePosition::SouthSouthWest:
            bearing = 202.5;
            break;
        case RelativePosition::SouthWest:
            bearing = 225.0;
            break;
        case RelativePosition::WestSouthWest:
            bearing = 247.5;
            break;
        case RelativePosition::West:
            bearing = 270.0;
            break;
        case RelativePosition::WestNorthWest:
            bearing = 292.5;
            break;
        case RelativePosition::NorthWest:
            bearing = 315.0;
            break;
        case RelativePosition::NorthNorthWest:
            bearing = 337.5;
            break;
        }
        bearing += positionData.trueHeading;
        SkyMath::Coordinate initial = SkyMath::relativePosition(sourcePosition, SkyMath::feetToMeters(altitude), bearing, SkyMath::feetToMeters(distance));

        initialPosition.latitude = initial.first;
        initialPosition.longitude = initial.second;
        initialPosition.altitude = altitude;

    } // positionData is not null

    return initialPosition;
}
