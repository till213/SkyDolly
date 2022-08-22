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
#ifndef AIRCRAFTSERVICE_H
#define AIRCRAFTSERVICE_H

#include <memory>
#include <vector>
#include <cstdint>
#include <cstddef>

#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include "../PersistenceLib.h"

struct AircraftServicePrivate;

class PERSISTENCE_API AircraftService
{
public:
    AircraftService() noexcept;
    ~AircraftService() noexcept;

    bool store(std::int64_t flightId, std::size_t sequenceNumber, Aircraft &aircraft) noexcept;
    bool deleteByIndex(int index) noexcept;
    std::vector<AircraftInfo> getAircraftInfos(std::int64_t flightId, bool *ok = nullptr) const noexcept;

    bool changeTimeOffset(Aircraft &aircraft, std::int64_t newOffset) noexcept;
    bool changeTailNumber(Aircraft &aircraft, const QString &tailNumber) noexcept;

private:
    const std::unique_ptr<AircraftServicePrivate> d;
};

#endif // AIRCRAFTSERVICE_H
