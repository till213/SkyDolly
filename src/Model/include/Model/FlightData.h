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
#ifndef FLIGHTDATA_H
#define FLIGHTDATA_H

#include <cstdint>
#include <vector>

#include <QString>
#include <QDateTime>

#include <Kernel/Const.h>
#include "Aircraft.h"
#include "FlightCondition.h"
#include "ModelLib.h"

struct MODEL_API FlightData final
{
    std::int64_t id {Const::InvalidId};
    QDateTime creationTime {QDateTime::currentDateTime()};
    QString title;
    QString description;
    FlightCondition flightCondition;
    std::vector<Aircraft> aircraft;
    int userAircraftIndex {Const::InvalidIndex};

    inline void clear(bool withOneAircraft) noexcept {
        id = Const::InvalidId;
        title.clear();
        description.clear();
        flightCondition.clear();
        if (aircraft.size() > 0) {
            const int aircraftCount = withOneAircraft ? 1 : 0;
            aircraft.resize(aircraftCount);
            userAircraftIndex = withOneAircraft ? 0 : Const::InvalidIndex;
        }
        // A flight always has at least one aircraft; unless
        // it is newly allocated (the aircraft is only added in the constructor body)
        // or cleared just before loading a flight
        if (aircraft.size() > 0) {
            aircraft.at(0).clear();
        }
    }

    using SizeType = std::vector<Aircraft>::size_type;
    inline SizeType count() const noexcept
    {
        return aircraft.size();
    }

    using Iterator = std::vector<Aircraft>::iterator;
    Iterator begin() noexcept
    {
        return aircraft.begin();
    }

    Iterator end() noexcept
    {
        return aircraft.end();
    }

    using ConstIterator = std::vector<Aircraft>::const_iterator;
    ConstIterator begin() const noexcept
    {
        return aircraft.begin();
    }

    ConstIterator end() const noexcept
    {
        return aircraft.end();
    }

    // OPERATORS

    Aircraft &operator[](std::size_t index) noexcept
    {
        return aircraft[index];
    }

    const Aircraft &operator[](std::size_t index) const noexcept
    {
        return aircraft[index];
    }
};

#endif // FLIGHTDATA_H
