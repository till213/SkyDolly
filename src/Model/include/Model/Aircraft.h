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
#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <memory>
#include <cstdint>

class QDateTime;

#include "ModelLib.h"
#include "AircraftInfo.h"
#include "Engine.h"

class Position;
class Attitude;
class PrimaryFlightControl;
class SecondaryFlightControl;
class AircraftHandle;
class Light;
class FlightPlan;
struct AircraftPrivate;

class MODEL_API Aircraft final
{
public:
    Aircraft(std::int64_t id = Const::InvalidId) noexcept;
    Aircraft(const Aircraft &rhs) = delete;
    Aircraft(Aircraft &&rhs) noexcept;
    Aircraft &operator=(const Aircraft &rhs) = delete;
    Aircraft &operator=(Aircraft &&rhs) noexcept;
    ~Aircraft();

    std::int64_t getId() const noexcept;
    void setId(std::int64_t id) noexcept;

    Position &getPosition() const noexcept;
    Attitude &getAttitude() const noexcept;
    Engine &getEngine() const noexcept;
    PrimaryFlightControl &getPrimaryFlightControl() const noexcept;
    SecondaryFlightControl &getSecondaryFlightControl() const noexcept;
    AircraftHandle &getAircraftHandle() const noexcept;
    Light &getLight() const noexcept;

    FlightPlan &getFlightPlan() const noexcept;
    AircraftInfo &getAircraftInfo() const noexcept;
    void setAircraftInfo(AircraftInfo aircraftInfo) noexcept;
    void setTailNumber(const QString &tailNumber) noexcept;

    std::int64_t getTimeOffset() const noexcept;
    void setTimeOffset(std::int64_t timeOffset) noexcept;
    void addTimeOffset(std::int64_t deltaOffset) noexcept;

    /*!
     * Returns the duration of this Aircraft's flight.
     *
     * \return the flight duartion [ms]
     */
    std::int64_t getDurationMSec() const noexcept;

    /*!
     * Returns whether this aircraft has at least one sampled Position.
     *
     * \return \c true if any sampled Position data is available; \c false else
     */
    bool hasRecording() const noexcept;

    void clear() noexcept;

    bool operator==(const Aircraft &rhs) const noexcept;
    bool operator!=(const Aircraft &rhs) const noexcept;

    /*!
     * Invalidates the duration, such that it gets updated the next time
     * #getDurationMSec is called.
     *
     * Explicitly call this method after an aircraft has been recorded or
     * the sampled data has been changed (added or removed).
     *
     * \sa getDurationMSec
     */
    void invalidateDuration() noexcept;

private:
    std::unique_ptr<AircraftPrivate> d;
};

#endif // AIRCRAFT_H
