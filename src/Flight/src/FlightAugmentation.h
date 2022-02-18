/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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

#include "FlightLib.h"

class Aircraft;
class FlightAugmentationPrivate;

/*!
 * Augments flight data with attitude and velocity, for instance.
 *
 * This is useful for imported flights where no such data is available
 * (for instance only position data is imported).
 */
class FLIGHT_API FlightAugmentation
{
public:
    enum struct Procedures: std::uint32_t
    {
        None = 0x0,
        TakeOff = 0x1,
        Landing = 0x2,
        All = 0xffffffff
    };

    enum struct Aspects: std::uint32_t
    {
        None = 0x0,
        Attitude = 0x1,
        Velocity = 0x2,
        AttitudeAndVelocity = Attitude | Velocity,
        Engine = 0x4,
        Light = 0x8,
        All = 0xffffffff
    };

    FlightAugmentation(Procedures procedures = Procedures::All, Aspects aspects = Aspects::All) noexcept;
    ~FlightAugmentation() noexcept;

    void setProcedures(Procedures procedures) noexcept;
    Procedures getProcedures() const noexcept;
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

inline FlightAugmentation::Procedures operator|(FlightAugmentation::Procedures a, FlightAugmentation::Procedures b) noexcept
{
    typedef std::underlying_type<FlightAugmentation::Procedures>::type EnumType;
    return static_cast<FlightAugmentation::Procedures>(static_cast<EnumType>(a) | static_cast<EnumType>(b));
}

inline FlightAugmentation::Procedures operator&(FlightAugmentation::Procedures a, FlightAugmentation::Procedures b) noexcept
{
    typedef std::underlying_type<FlightAugmentation::Procedures>::type EnumType;
    return static_cast<FlightAugmentation::Procedures>(static_cast<EnumType>(a) & static_cast<EnumType>(b));
}

inline FlightAugmentation::Aspects operator|(FlightAugmentation::Aspects a, FlightAugmentation::Aspects b) noexcept
{
    typedef std::underlying_type<FlightAugmentation::Aspects>::type EnumType;
    return static_cast<FlightAugmentation::Aspects>(static_cast<EnumType>(a) | static_cast<EnumType>(b));
}

inline FlightAugmentation::Aspects operator&(FlightAugmentation::Aspects a, FlightAugmentation::Aspects b) noexcept
{
    typedef std::underlying_type<FlightAugmentation::Aspects>::type EnumType;
    return static_cast<FlightAugmentation::Aspects>(static_cast<EnumType>(a) & static_cast<EnumType>(b));
}

inline bool operator==(FlightAugmentation::Aspects a, FlightAugmentation::Aspects b) noexcept
{
    typedef std::underlying_type<FlightAugmentation::Aspects>::type EnumType;
    return static_cast<EnumType>(a) == static_cast<EnumType>(b);
}

#endif // FLIGHTAUGMENTATION_H
