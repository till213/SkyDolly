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
#include <memory>
#include <algorithm>
#include <cstdint>
#include <utility>

#include <Kernel/Convert.h>
#include <Kernel/SkyMath.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/AircraftHandle.h>
#include <Model/AircraftHandleData.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/SecondaryFlightControl.h>
#include <Model/SecondaryFlightControlData.h>
#include <Model/Light.h>
#include <Model/LightData.h>
#include "Analytics.h"
#include "FlightAugmentation.h"

/*! \todo IMPLEMENT ME: Those are the typical/max values for A320-like aircraft
 *        -> define "per aircraft group"-specific limits (jet, propeller/GA, turboprop, glider, ...)
 */
namespace {

    // Estimated landing speed [knots]
    constexpr double LandingVelocity = 140.0;
    // Estimated landing pitch [degrees]
    // Note: negative pitch values mean "noise points upwards"
    constexpr double LandingPitch = -3.0;
    // Max banking angle [degrees]
    // https://www.pprune.org/tech-log/377244-a320-321-ap-bank-angle-limits.html
    constexpr double MaxBankAngle = 25;
}

struct FlightAugmentationPrivate
{
    FlightAugmentationPrivate(FlightAugmentation::Procedures theProcedures, FlightAugmentation::Aspects theAspects)
        : procedures(theProcedures),
          aspects(theAspects)
    {}

    FlightAugmentation::Procedures procedures;
    FlightAugmentation::Aspects aspects;
};

// PUBLIC

FlightAugmentation::FlightAugmentation(Procedures procedures, Aspects aspects) noexcept
    : d(std::make_unique<FlightAugmentationPrivate>(procedures, aspects))
{}

FlightAugmentation::FlightAugmentation(FlightAugmentation &&rhs) noexcept = default;
FlightAugmentation &FlightAugmentation::operator=(FlightAugmentation &&rhs) noexcept = default;
FlightAugmentation::~FlightAugmentation() = default;

void FlightAugmentation::setProcedures(Procedures procedures) noexcept
{
    d->procedures = procedures;
}

FlightAugmentation::Procedures FlightAugmentation::getProcedures() const noexcept
{
    return d->procedures;
}

void FlightAugmentation::setAspects(Aspects aspects) noexcept
{
    d->aspects = aspects;
}

FlightAugmentation::Aspects FlightAugmentation::getAspects() const noexcept
{
    return d->aspects;
}

void FlightAugmentation::augmentAircraftData(Aircraft &aircraft) noexcept
{
    if (d->aspects) {
        augmentAttitudeAndVelocity(aircraft);
    }

    if (d->procedures) {
        augmentProcedures(aircraft);
    }
}

void FlightAugmentation::augmentAttitudeAndVelocity(Aircraft &aircraft) noexcept
{
    Position &position = aircraft.getPosition();
    const auto positionCount = position.count();

    Analytics analytics(aircraft);
    const auto [firstMovementTimestamp, firstMovementHeading] = analytics.firstMovementHeading();

    for (int i = 0; i < positionCount; ++i) {
        if (i < positionCount - 1) {

            PositionData &startPositionData = position[i];
            const PositionData &endPositionData = position[i + 1];
            const SkyMath::Coordinate startPosition(startPositionData.latitude, startPositionData.longitude);
            const std::int64_t startTimestamp = startPositionData.timestamp;
            const SkyMath::Coordinate endPosition(endPositionData.latitude, endPositionData.longitude);
            const std::int64_t endTimestamp = endPositionData.timestamp;

            const auto [distance, speed] = SkyMath::distanceAndSpeed(startPosition, startTimestamp, endPosition, endTimestamp);
            // Velocity
            if (d->aspects.testFlag(Aspect::Velocity)) {
                startPositionData.velocityBodyX = 0.0;
                startPositionData.velocityBodyY = 0.0;
                startPositionData.velocityBodyZ = Convert::metersPerSecondToFeetPerSecond(speed);
            }

            // Attitude
            if ((d->aspects & Aspect::Attitude)) {
                if (startPositionData.timestamp > firstMovementTimestamp) {
                    const double deltaAltitude = Convert::feetToMeters(endPositionData.altitude - startPositionData.altitude);
                    // SimConnect: positive pitch values "point downwards", negative pitch values "upwards"
                    // -> switch the sign
                    if (d->aspects.testFlag(Aspect::Pitch)) {
                        startPositionData.pitch = -SkyMath::approximatePitch(distance, deltaAltitude);
                    }
                    const double initialBearing = SkyMath::initialBearing(startPosition, endPosition);
                    if (d->aspects.testFlag(Aspect::Heading)) {
                        startPositionData.trueHeading = initialBearing;
                    }
                    if (d->aspects.testFlag(Aspect::Bank)) {
                        if (i > 0) {
                            // [-180, 180]
                            const double headingChange = SkyMath::headingChange(position[i - 1].trueHeading, startPositionData.trueHeading);
                            // We go into maximum bank angle of 30 degrees with a heading change of 45 degrees
                            // SimConnect: negative values are a "right" turn, positive values a left turn
                            startPositionData.bank = SkyMath::bankAngle(headingChange, 45.0, ::MaxBankAngle);
                        } else {
                            // First point, zero bank angle
                            startPositionData.bank = 0.0;
                        }
                    }
                } else {
                    if (d->aspects.testFlag(Aspect::Pitch)) {
                        startPositionData.pitch = 0.0;
                    }
                    if (d->aspects.testFlag(Aspect::Heading)) {
                        startPositionData.trueHeading = firstMovementHeading;
                    }
                    if (d->aspects.testFlag(Aspect::Bank)) {
                        startPositionData.bank = 0.0;
                    }
                }
            }

        } else if (positionCount > 1) {
            // Last point
            PositionData &lastPositionData = position[i];

            // Velocity
            PositionData &previousPositionData = position[i -1];
            if (d->aspects.testFlag(Aspect::Velocity)) {
                lastPositionData.velocityBodyX = previousPositionData.velocityBodyX;
                lastPositionData.velocityBodyY = previousPositionData.velocityBodyY;
                lastPositionData.velocityBodyZ = Convert::knotsToFeetPerSecond(::LandingVelocity);
            }

            // Attitude
            if ((d->aspects & Aspect::Attitude)) {
                if (d->aspects.testFlag(Aspect::Pitch)) {
                    lastPositionData.pitch = ::LandingPitch;
                }
                if (d->aspects.testFlag(Aspect::Bank)) {
                    lastPositionData.bank = 0.0;
                }
                if (d->aspects.testFlag(Aspect::Heading)) {
                    lastPositionData.trueHeading = previousPositionData.trueHeading;
                }
            }
        } else {
            // Only one sampled data point ("academic case")
            PositionData &lastPositionData = position[i];

            // Velocity
            if (d->aspects.testFlag(Aspect::Velocity)) {
                lastPositionData.velocityBodyX = 0.0;
                lastPositionData.velocityBodyY = 0.0;
                lastPositionData.velocityBodyZ = 0.0;
            }

            // Attitude
            if ((d->aspects & Aspect::Attitude)) {
                if (d->aspects.testFlag(Aspect::Pitch)) {
                    lastPositionData.pitch = 0.0;
                }
                if (d->aspects.testFlag(Aspect::Bank)) {
                    lastPositionData.bank = 0.0;
                }
                if (d->aspects.testFlag(Aspect::Heading)) {
                    lastPositionData.trueHeading = 0.0;
                }
            }
        }
    }
}

void FlightAugmentation::augmentProcedures(Aircraft &aircraft) noexcept
{
    augmentStartProcedure(aircraft);
    augmentLandingProcedure(aircraft);

    // In case the flight is very short it is possible that the augmented start- and landing
    // events overlap and are hence out of order
    std::sort(aircraft.getEngine().begin(), aircraft.getEngine().end());
    std::sort(aircraft.getSecondaryFlightControl().begin(), aircraft.getSecondaryFlightControl().end());
    std::sort(aircraft.getAircraftHandle().begin(), aircraft.getAircraftHandle().end());
    std::sort(aircraft.getLight().begin(), aircraft.getLight().end());
}

// PRIVATE

void FlightAugmentation::augmentStartProcedure(Aircraft &aircraft) noexcept
{
    const std::int64_t lastTimestamp = aircraft.getPosition().getLast().timestamp;

    if (d->aspects.testFlag(Aspect::Engine)) {

        // Engine

        Engine &engine = aircraft.getEngine();
        EngineData engineData;

        // 0 seconds
        engineData.timestamp = 0;
        engineData.electricalMasterBattery1 = true;
        engineData.electricalMasterBattery2 = true;
        engineData.electricalMasterBattery3 = true;
        engineData.electricalMasterBattery4 = true;
        engineData.generalEngineCombustion1 = true;
        engineData.generalEngineCombustion2 = true;
        engineData.generalEngineCombustion3 = true;
        engineData.generalEngineCombustion4 = true;
        engineData.throttleLeverPosition1 = SkyMath::fromNormalisedPosition(1.0);
        engineData.throttleLeverPosition2 = SkyMath::fromNormalisedPosition(1.0);
        engineData.throttleLeverPosition3 = SkyMath::fromNormalisedPosition(1.0);
        engineData.throttleLeverPosition4 = SkyMath::fromNormalisedPosition(1.0);
        engineData.propellerLeverPosition1 = SkyMath::fromNormalisedPosition(1.0);
        engineData.propellerLeverPosition2 = SkyMath::fromNormalisedPosition(1.0);
        engineData.propellerLeverPosition3 = SkyMath::fromNormalisedPosition(1.0);
        engineData.propellerLeverPosition4 = SkyMath::fromNormalisedPosition(1.0);
        engineData.mixtureLeverPosition1 = SkyMath::fromPercent(100.0);
        engineData.mixtureLeverPosition2 = SkyMath::fromPercent(100.0);
        engineData.mixtureLeverPosition3 = SkyMath::fromPercent(100.0);
        engineData.mixtureLeverPosition4 = SkyMath::fromPercent(100.0);
        // The start procedure is the first procedure, and elements
        // are inserted chronologically from the start, so we can
        // use upsertLast (instead of the more general upsert)
        engine.upsertLast(engineData);

        // 2 minutes
        engineData.timestamp = std::min(static_cast<std::int64_t>(2 * 60 * 1000), lastTimestamp);
        engineData.electricalMasterBattery1 = true;
        engineData.electricalMasterBattery2 = true;
        engineData.electricalMasterBattery3 = true;
        engineData.electricalMasterBattery4 = true;
        engineData.generalEngineCombustion1 = true;
        engineData.generalEngineCombustion2 = true;
        engineData.generalEngineCombustion3 = true;
        engineData.generalEngineCombustion4 = true;
        // In the (stock) A320neo 86% correspond to "climb" throttle detent
        engineData.throttleLeverPosition1 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition2 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition3 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition4 = SkyMath::fromNormalisedPosition(0.86);
        // Reduce propeller power to 80%
        engineData.propellerLeverPosition1 = SkyMath::fromNormalisedPosition(0.80);
        engineData.propellerLeverPosition2 = SkyMath::fromNormalisedPosition(0.80);
        engineData.propellerLeverPosition3 = SkyMath::fromNormalisedPosition(0.80);
        engineData.propellerLeverPosition4 = SkyMath::fromNormalisedPosition(0.80);
        // Mixture down to 85%
        engineData.mixtureLeverPosition1 = SkyMath::fromPercent(85.0);
        engineData.mixtureLeverPosition2 = SkyMath::fromPercent(85.0);
        engineData.mixtureLeverPosition3 = SkyMath::fromPercent(85.0);
        engineData.mixtureLeverPosition4 = SkyMath::fromPercent(85.0);
        engine.upsertLast(engineData);

        // 5 minutes
        engineData.timestamp = std::min(static_cast<std::int64_t>(5 * 60 * 1000), lastTimestamp);
        engineData.electricalMasterBattery1 = true;
        engineData.electricalMasterBattery2 = true;
        engineData.electricalMasterBattery3 = true;
        engineData.electricalMasterBattery4 = true;
        engineData.generalEngineCombustion1 = true;
        engineData.generalEngineCombustion2 = true;
        engineData.generalEngineCombustion3 = true;
        engineData.generalEngineCombustion4 = true;
        engineData.throttleLeverPosition1 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition2 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition3 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition4 = SkyMath::fromNormalisedPosition(0.86);
        engineData.propellerLeverPosition1 = SkyMath::fromNormalisedPosition(0.80);
        engineData.propellerLeverPosition2 = SkyMath::fromNormalisedPosition(0.80);
        engineData.propellerLeverPosition3 = SkyMath::fromNormalisedPosition(0.80);
        engineData.propellerLeverPosition4 = SkyMath::fromNormalisedPosition(0.80);
        // Mixture down to 75%
        engineData.mixtureLeverPosition1 = SkyMath::fromPercent(75.0);
        engineData.mixtureLeverPosition2 = SkyMath::fromPercent(75.0);
        engineData.mixtureLeverPosition3 = SkyMath::fromPercent(75.0);
        engineData.mixtureLeverPosition4 = SkyMath::fromPercent(75.0);
        engine.upsertLast(engineData);

    }

    // Secondary flight controls

    SecondaryFlightControl &secondaryFlightControl = aircraft.getSecondaryFlightControl();
    SecondaryFlightControlData secondaryFlightControlData;

    // 0 seconds
    secondaryFlightControlData.timestamp = 0;
    // Flaps
    secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.666);
    secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.666);
    secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.286);
    secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.286);
    secondaryFlightControlData.flapsHandleIndex = 1;
    secondaryFlightControlData.spoilersHandlePosition = 0;
    secondaryFlightControl.upsertLast(secondaryFlightControlData);

    // 30 seconds
    secondaryFlightControlData.timestamp = std::min(static_cast<std::int64_t>(30 * 1000), lastTimestamp);
    // Retract flaps
    secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.0);
    secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.0);
    secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.0);
    secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.0);
    secondaryFlightControlData.flapsHandleIndex = 0;
    secondaryFlightControlData.spoilersHandlePosition = 0;
    secondaryFlightControl.upsertLast(secondaryFlightControlData);

    // Handles & gear

    AircraftHandle &aircraftHandle = aircraft.getAircraftHandle();
    AircraftHandleData handleData;

    // 0 seconds
    handleData.timestamp = 0;
    // Gear down
    handleData.gearHandlePosition = true;
    aircraftHandle.upsertLast(handleData);

    // 5 seconds
    handleData.timestamp = std::min(static_cast<std::int64_t>(5 * 1000), lastTimestamp);
    // Gear up
    handleData.gearHandlePosition = false;
    aircraftHandle.upsertLast(handleData);

    // Lights

    Light &light = aircraft.getLight();
    LightData lightData;

    // 0 seconds
    lightData.timestamp = 0;
    lightData.lightStates = SimType::LightState::Navigation |
                            SimType::LightState::Beacon |
                            SimType::LightState::Landing |
                            SimType::LightState::Strobe |
                            SimType::LightState::Panel |
                            SimType::LightState::Recognition |
                            SimType::LightState::Wing |
                            SimType::LightState::Logo;
    light.upsertLast(lightData);

    // 3 minutes
    lightData.timestamp = std::min(static_cast<std::int64_t>(3 * 60 * 1000), lastTimestamp);
    lightData.lightStates = SimType::LightState::Navigation |
                            SimType::LightState::Beacon |
                            SimType::LightState::Strobe |
                            SimType::LightState::Panel |
                            SimType::LightState::Recognition |
                            SimType::LightState::Wing |
                            SimType::LightState::Logo;
    light.upsertLast(lightData);

    // 4 minutes
    lightData.timestamp = std::min(static_cast<std::int64_t>(4 * 60 * 1000), lastTimestamp);
    lightData.lightStates = SimType::LightState::Navigation |
                            SimType::LightState::Beacon |
                            SimType::LightState::Strobe |
                            SimType::LightState::Panel |
                            SimType::LightState::Recognition |
                            SimType::LightState::Logo;
    light.upsertLast(lightData);
}

/*! \todo: Calculate times based on the following rule of thumb:
 * flaps 1 at 10miles, flaps 2 at 8 miles, gear down between 6-7miles,
 * flaps 3 at 5 miles followed by full flaps almost immediately afterwards.
 */
void FlightAugmentation::augmentLandingProcedure(Aircraft &aircraft) noexcept
{
    Position &position = aircraft.getPosition();
    const std::int64_t lastTimestamp = position.getLast().timestamp;

    // Engine
    if (d->aspects.testFlag(Aspect::Engine)) {
        Engine &engine = aircraft.getEngine();
        EngineData engineData;

        // t minus 5 minutes
        engineData.timestamp = std::max(lastTimestamp - std::int64_t(5 * 60 * 1000), std::int64_t(0));
        engineData.electricalMasterBattery1 = true;
        engineData.electricalMasterBattery2 = true;
        engineData.electricalMasterBattery3 = true;
        engineData.electricalMasterBattery4 = true;
        engineData.generalEngineCombustion1 = true;
        engineData.generalEngineCombustion2 = true;
        engineData.generalEngineCombustion3 = true;
        engineData.generalEngineCombustion4 = true;
        engineData.throttleLeverPosition1 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition2 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition3 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition4 = SkyMath::fromNormalisedPosition(0.86);
        engineData.propellerLeverPosition1 = SkyMath::fromNormalisedPosition(0.60);
        engineData.propellerLeverPosition2 = SkyMath::fromNormalisedPosition(0.60);
        engineData.propellerLeverPosition3 = SkyMath::fromNormalisedPosition(0.60);
        engineData.propellerLeverPosition4 = SkyMath::fromNormalisedPosition(0.60);
        // Mixture up to 85%
        engineData.mixtureLeverPosition1 = SkyMath::fromPercent(85.0);
        engineData.mixtureLeverPosition2 = SkyMath::fromPercent(85.0);
        engineData.mixtureLeverPosition3 = SkyMath::fromPercent(85.0);
        engineData.mixtureLeverPosition4 = SkyMath::fromPercent(85.0);
        engine.upsert(engineData);

        // t minus 2 minutes
        engineData.timestamp = std::max(lastTimestamp - std::int64_t(2 * 60 * 1000), std::int64_t(0));
        engineData.electricalMasterBattery1 = true;
        engineData.electricalMasterBattery2 = true;
        engineData.electricalMasterBattery3 = true;
        engineData.electricalMasterBattery4 = true;
        engineData.generalEngineCombustion1 = true;
        engineData.generalEngineCombustion2 = true;
        engineData.generalEngineCombustion3 = true;
        engineData.generalEngineCombustion4 = true;
        engineData.throttleLeverPosition1 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition2 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition3 = SkyMath::fromNormalisedPosition(0.86);
        engineData.throttleLeverPosition4 = SkyMath::fromNormalisedPosition(0.86);
        // Propeller down to 40%
        engineData.propellerLeverPosition1 = SkyMath::fromNormalisedPosition(0.40);
        engineData.propellerLeverPosition2 = SkyMath::fromNormalisedPosition(0.40);
        engineData.propellerLeverPosition3 = SkyMath::fromNormalisedPosition(0.40);
        engineData.propellerLeverPosition4 = SkyMath::fromNormalisedPosition(0.40);
        // Mixture up to 100%
        engineData.mixtureLeverPosition1 = SkyMath::fromPercent(100.0);
        engineData.mixtureLeverPosition2 = SkyMath::fromPercent(100.0);
        engineData.mixtureLeverPosition3 = SkyMath::fromPercent(100.0);
        engineData.mixtureLeverPosition4 = SkyMath::fromPercent(100.0);
        engine.upsert(engineData);

        // At end
        engineData.timestamp = lastTimestamp;
        engineData.electricalMasterBattery1 = true;
        engineData.electricalMasterBattery2 = true;
        engineData.electricalMasterBattery3 = true;
        engineData.electricalMasterBattery4 = true;
        engineData.generalEngineCombustion1 = true;
        engineData.generalEngineCombustion2 = true;
        engineData.generalEngineCombustion3 = true;
        engineData.generalEngineCombustion4 = true;
        // Reverse thrust (-20%)
        engineData.throttleLeverPosition1 = SkyMath::fromNormalisedPosition(-0.2);
        engineData.throttleLeverPosition2 = SkyMath::fromNormalisedPosition(-0.2);
        engineData.throttleLeverPosition3 = SkyMath::fromNormalisedPosition(-0.2);
        engineData.throttleLeverPosition4 = SkyMath::fromNormalisedPosition(-0.2);
        // Propeller down to 0%
        engineData.propellerLeverPosition1 = SkyMath::fromNormalisedPosition(0.0);
        engineData.propellerLeverPosition2 = SkyMath::fromNormalisedPosition(0.0);
        engineData.propellerLeverPosition3 = SkyMath::fromNormalisedPosition(0.0);
        engineData.propellerLeverPosition4 = SkyMath::fromNormalisedPosition(0.0);
        engineData.mixtureLeverPosition1 = SkyMath::fromPercent(100.0);
        engineData.mixtureLeverPosition2 = SkyMath::fromPercent(100.0);
        engineData.mixtureLeverPosition3 = SkyMath::fromPercent(100.0);
        engineData.mixtureLeverPosition4 = SkyMath::fromPercent(100.0);
        engine.upsert(engineData);
    }

    // Secondary flight controls

    SecondaryFlightControl &secondaryFlightControl = aircraft.getSecondaryFlightControl();
    SecondaryFlightControlData secondaryFlightControlData;

    // t minus 10 minutes
    secondaryFlightControlData.timestamp = std::max(lastTimestamp - std::int64_t(10 * 60 * 1000), std::int64_t(0));
    // Flaps 0
    secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.0);
    secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.0);
    secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.0);
    secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.0);
    secondaryFlightControlData.flapsHandleIndex = 0;
    // Spoilers 40%
    secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(20.0);
    secondaryFlightControl.upsert(secondaryFlightControlData);

    // t minus 8 minutes
    secondaryFlightControlData.timestamp = std::max(lastTimestamp - std::int64_t(8 * 60 * 1000), std::int64_t(0));
    // Flaps 1
    secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.666);
    secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.666);
    secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.286);
    secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.286);
    secondaryFlightControlData.flapsHandleIndex = 1;
    // Spoilers 60%
    secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(60.0);
    secondaryFlightControl.upsert(secondaryFlightControlData);

    // t minus 7 minutes
    secondaryFlightControlData.timestamp = std::max(lastTimestamp - std::int64_t(7 * 60 * 1000), std::int64_t(0));
    // Flaps 2
    secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.8157);
    secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.8157);
    secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.4275);
    secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.4275);
    secondaryFlightControlData.flapsHandleIndex = 2;
    // Spoilers 60%
    secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(60.0);
    secondaryFlightControl.upsert(secondaryFlightControlData);

    // t minus 5 minutes
    secondaryFlightControlData.timestamp = std::max(lastTimestamp - std::int64_t(5 * 60 * 1000), std::int64_t(0));
    // Flaps 3
    secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.8157);
    secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.8157);
    secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(0.5725);
    secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(0.5725);
    secondaryFlightControlData.flapsHandleIndex = 3;
    // Spoilers 20%
    secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(20.0);
    secondaryFlightControl.upsert(secondaryFlightControlData);

    // t minus 4 minutes
    secondaryFlightControlData.timestamp = std::max(lastTimestamp - std::int64_t(4 * 60 * 1000), std::int64_t(0));
    // Flaps 4
    secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(1.0);
    secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(1.0);
    secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(1.0);
    secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(1.0);
    secondaryFlightControlData.flapsHandleIndex = 4;
    // Spoilers 0%
    secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(0.0);
    secondaryFlightControl.upsert(secondaryFlightControlData);

    // t
    secondaryFlightControlData.timestamp = lastTimestamp;
    // Flaps 4
    secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(1.0);
    secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(1.0);
    secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(1.0);
    secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(1.0);
    secondaryFlightControlData.flapsHandleIndex = 4;
    // Spoilers 100%
    secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(100.0);
    secondaryFlightControl.upsert(secondaryFlightControlData);

    // Handles & gear

    AircraftHandle &aircraftHandle = aircraft.getAircraftHandle();
    AircraftHandleData handleData;

    // t minus 3 minutes
    handleData.timestamp = std::max(lastTimestamp - std::int64_t(3 * 60 * 1000), std::int64_t(0));
    // Gear down
    handleData.gearHandlePosition = true;
    aircraftHandle.upsert(handleData);

    // Lights
    if (d->aspects.testFlag(Aspect::Light)) {

        Light &light = aircraft.getLight();
        LightData lightData;

        // t minus 8 minutes
        lightData.timestamp = std::max(lastTimestamp - std::int64_t(8 * 60 * 1000), std::int64_t(0));
        lightData.lightStates = SimType::LightState::Navigation |
                                SimType::LightState::Beacon |
                                SimType::LightState::Strobe |
                                SimType::LightState::Panel |
                                SimType::LightState::Recognition |
                                SimType::LightState::Wing |
                                SimType::LightState::Logo;
        light.upsert(lightData);

        // t minus 6 minutes
        lightData.timestamp = std::max(lastTimestamp - std::int64_t(6 * 60 * 1000), std::int64_t(0));
        lightData.lightStates = SimType::LightState::Navigation |
                                SimType::LightState::Beacon |
                                SimType::LightState::Landing |
                                SimType::LightState::Strobe |
                                SimType::LightState::Panel |
                                SimType::LightState::Recognition |
                                SimType::LightState::Wing |
                                SimType::LightState::Logo;
        light.upsert(lightData);

        // t minus 4 minutes
        lightData.timestamp = std::max(lastTimestamp - std::int64_t(4 * 60 * 1000), std::int64_t(0));
        lightData.lightStates = SimType::LightState::Navigation |
                                SimType::LightState::Beacon |
                                SimType::LightState::Landing |
                                SimType::LightState::Taxi |
                                SimType::LightState::Strobe |
                                SimType::LightState::Panel |
                                SimType::LightState::Recognition |
                                SimType::LightState::Wing |
                                SimType::LightState::Logo;
        light.upsert(lightData);
    }

    // Adjust approach pitch for the last 3 minutes
    // https://forum.aerosoft.com/index.php?/topic/123864-a320-pitch-angle-during-landing/
    if (d->aspects.testFlag(Aspect::Pitch)) {
        auto index = position.count() - 1;
        if (index >= 0) {
            // Last sample: flare with nose up 6 degrees
            PositionData &positionData = position[index];
            positionData.pitch = -6.0;

            if (index > 0) {
                // Second to last sample -> adjust pitch to 3 degrees nose up
                --index;
                while (index >= 0 && position[index].timestamp >= std::max(lastTimestamp - std::int64_t(3 * 60 * 1000), std::int64_t(0))) {
                    // Nose up 3 degrees
                    position[index].pitch = -3.0;
                    --index;
                }
            }
        }
    }
}
