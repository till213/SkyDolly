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
#include <utility>

#include <Kernel/Convert.h>
#include <Kernel/SkyMath.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Attitude.h>
#include <Model/AttitudeData.h>
#include "Formation.h"

InitialPosition Formation::calculateInitialRelativePositionToUserAircraft(HorizontalDistance horizontalDistance, VerticalDistance verticalDistance, Bearing bearing, std::int64_t timestamp) noexcept
{
    InitialPosition initialPosition;

    const auto relativePosition = calculateRelativePositionToUserAircraft(horizontalDistance, verticalDistance, bearing, timestamp);
    if (!relativePosition.first.isNull()) {
        initialPosition.fromPositionData(relativePosition.first, relativePosition.second);
    }
    return initialPosition;
}

std::pair<PositionData, AttitudeData> Formation::calculateRelativePositionToUserAircraft(HorizontalDistance horizontalDistance, VerticalDistance verticalDistance, Bearing bearing, std::int64_t timestamp) noexcept
{
    PositionData initialPositionData;
    AttitudeData attitudeData;

    const auto &flight = Logbook::getInstance().getCurrentFlight();
    const auto &aircraft = flight.getUserAircraft();
    const auto &position = aircraft.getPosition();
    if (position.count() > 0) {
        const auto &positionData = position.interpolate(timestamp, TimeVariableData::Access::DiscreteSeek);
        const auto &aircraftInfo = aircraft.getAircraftInfo();
        const auto &aircraftType = aircraftInfo.aircraftType;

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
        double bearingDegrees {0.0};
        switch (bearing) {
        case Bearing::North:
            bearingDegrees = 0.0;
            break;
        case Bearing::NorthNorthEast:
            bearingDegrees = 22.5;
            break;
        case Bearing::NorthEast:
            bearingDegrees = 45.0;
            break;
        case Bearing::EastNorthEast:
            bearingDegrees = 67.5;
            break;
        case Bearing::East:
            bearingDegrees = 90.0;
            break;
        case Bearing::EastSouthEast:
            bearingDegrees = 112.5;
            break;
        case Bearing::SouthEast:
            bearingDegrees = 135.0;
            break;
        case Bearing::SouthSouthEast:
            bearingDegrees = 157.5;
            break;
        case Bearing::South:
            bearingDegrees = 180.0;
            break;
        case Bearing::SouthSouthWest:
            bearingDegrees = 202.5;
            break;
        case Bearing::SouthWest:
            bearingDegrees = 225.0;
            break;
        case Bearing::WestSouthWest:
            bearingDegrees = 247.5;
            break;
        case Bearing::West:
            bearingDegrees = 270.0;
            break;
        case Bearing::WestNorthWest:
            bearingDegrees = 292.5;
            break;
        case Bearing::NorthWest:
            bearingDegrees = 315.0;
            break;
        case Bearing::NorthNorthWest:
            bearingDegrees = 337.5;
            break;
        }
        attitudeData = aircraft.getAttitude().interpolate(timestamp, TimeVariableData::Access::DiscreteSeek);
        bearingDegrees += attitudeData.trueHeading;
        SkyMath::Coordinate coordinate = SkyMath::relativePosition(sourcePosition, bearingDegrees, Convert::feetToMeters(distance));

        initialPositionData.timestamp = positionData.timestamp;
        initialPositionData.latitude = coordinate.first;
        initialPositionData.longitude = coordinate.second;
        initialPositionData.initialiseCommonAltitude(altitude);

    } // position count > 0

    return std::make_pair(initialPositionData, attitudeData);
}
