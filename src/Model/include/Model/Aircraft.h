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
#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <memory>
#include <cstdint>

#include <QObject>

class QDateTime;

#include "ModelLib.h"
#include "AircraftInfo.h"
#include "Engine.h"

class Position;
class PrimaryFlightControl;
class SecondaryFlightControl;
class AircraftHandle;
class Light;
class FlightPlan;
class AircraftPrivate;

class MODEL_API Aircraft : public QObject
{
    Q_OBJECT
public:

    static constexpr std::int64_t InvalidId = -1;
    static constexpr std::int64_t InvalidSimulationId = -1;
    static constexpr std::int64_t PendingSimulationId = -2;

    Aircraft(QObject *parent = nullptr) noexcept;
    virtual ~Aircraft() noexcept;

    std::int64_t getId() const noexcept;
    void setId(std::int64_t id) noexcept;

    std::int64_t getSimulationObjectId() const noexcept;
    void setSimulationObjectId(std::int64_t id) noexcept;

    const Position &getPositionConst() const noexcept;
    Position &getPosition() const noexcept;

    const Engine &getEngineConst() const noexcept;
    Engine &getEngine() const noexcept;

    const PrimaryFlightControl &getPrimaryFlightControlConst() const noexcept;
    PrimaryFlightControl &getPrimaryFlightControl() const noexcept;

    const SecondaryFlightControl &getSecondaryFlightControlConst() const noexcept;
    SecondaryFlightControl &getSecondaryFlightControl() const noexcept;

    const AircraftHandle &getAircraftHandleConst() const noexcept;
    AircraftHandle &getAircraftHandle() const noexcept;

    const Light &getLightConst() const noexcept;
    Light &getLight() const noexcept;

    const AircraftInfo &getAircraftInfoConst() const noexcept;
    void setAircraftInfo(const AircraftInfo &aircraftInfo) noexcept;

    void setTailNumber(const QString &tailNumber) noexcept;

    std::int64_t getTimeOffset() const noexcept;
    void setTimeOffset(std::int64_t timeOffset) noexcept;

    const FlightPlan &getFlightPlanConst() const noexcept;
    FlightPlan &getFlightPlan() const noexcept;

    std::int64_t getDurationMSec() const noexcept;
    bool hasRecording() const noexcept;

    void clear() noexcept;

    bool operator == (const Aircraft &rhs) const noexcept;
    bool operator != (const Aircraft &rhs) const noexcept;

signals:
    void infoChanged(Aircraft &aircraft);
    void dataChanged();
    void tailNumberChanged(Aircraft &aircraft);
    void timeOffsetChanged(Aircraft &aircraft);

private:
    Q_DISABLE_COPY(Aircraft)
    std::unique_ptr<AircraftPrivate> d;

    void frenchConnection();

private slots:
    void handleDataChanged();
    void invalidateDuration();
};

#endif // AIRCRAFT_H
