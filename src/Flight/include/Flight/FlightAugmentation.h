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
#ifndef FLIGHTAUGMENTATION_H
#define FLIGHTAUGMENTATION_H

#include <memory>
#include <cstdint>

#include <QFlags>

#include "FlightLib.h"

class Aircraft;
struct FlightAugmentationPrivate;

/*!
 * Augments flight data with attitude and velocity, for instance.
 *
 * This is useful for imported flights where no such data is available
 * (for instance only position data is imported).
 */
class FLIGHT_API FlightAugmentation
{
public:
    enum struct Procedure: std::uint32_t
    {
        None = 0x0,
        TakeOff = 0x1,
        Landing = 0x2,
        All = 0xffffffff
    };
    Q_DECLARE_FLAGS(Procedures, Procedure)

    enum struct Aspect: std::uint32_t
    {
        None = 0x0,
        Pitch = 0x1,
        Bank = 0x2,
        Heading = 0x4,
        Attitude = Pitch | Bank | Heading,
        Velocity = 0x8,
        AttitudeAndVelocity = Attitude | Velocity,
        Engine = 0x10,
        Light = 0x20,
        All = 0xffffffff
    };
    Q_DECLARE_FLAGS(Aspects, Aspect)

    FlightAugmentation(Procedures procedures = Procedure::All, Aspects aspects = Aspect::All) noexcept;
    FlightAugmentation(const FlightAugmentation &rhs) = delete;
    FlightAugmentation(FlightAugmentation &&rhs) noexcept;
    FlightAugmentation &operator=(const  FlightAugmentation &rhs) = delete;
    FlightAugmentation &operator=(FlightAugmentation &&rhs) noexcept;
    ~FlightAugmentation();

    void setProcedures(Procedures procedures) noexcept;
    Procedures getAugmentationProcedures() const noexcept;
    void setAspects(Aspects aspects) noexcept;
    Aspects getAspects() const noexcept;

    void augmentAircraftData(Aircraft &aircraft) noexcept;
    void augmentAttitudeAndVelocity(Aircraft &aircraft) noexcept;
    void augmentProcedures(Aircraft &aircraft) noexcept;

private:
    std::unique_ptr<FlightAugmentationPrivate> d;

    void augmentStartProcedure(Aircraft &aircraft) noexcept;
    void augmentLandingProcedure(Aircraft &aircraft) noexcept;
};

#endif // FLIGHTAUGMENTATION_H
