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
#include "../../Kernel/src/SkyMath.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/Position.h"
#include "../../Model/src/PositionData.h"
#include "Analytics.h"

class AnalyticsPrivate
{
public:
    AnalyticsPrivate(const Aircraft &theAircraft)
        : aircraft(theAircraft)
    {}

    const Aircraft &aircraft;
};

// PUBLIC

Analytics::Analytics(const Aircraft &aircraft)
    : d(std::make_unique<AnalyticsPrivate>(aircraft))
{}

Analytics::~Analytics()
{}

const PositionData &Analytics::firstMovementPosition() const noexcept
{
    // @todo IMPLEMENT ME!!!
    Position &position = d->aircraft.getPosition();
    const PositionData &positionData = position.getFirst();
    const std::size_t count = position.count();
    auto curr = position.begin();
    auto prev = curr;
    if (count > 1) {
        auto pos = std::adjacent_find(position.begin(), position.end(),
                                     [](const PositionData &a, const PositionData &b)->bool { return std::abs(a.longitude - b.longitude) > 0.001;});
        qDebug("Timestamp %llu", pos->timestamp);

    }

    return positionData;
}
